#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <map>
#include <set>
#include <vector>
#include <memory>
#include <stdexcept>
//tymczasowe
#include <iostream>

struct VirusAlreadyCreated : public std::invalid_argument {
    VirusAlreadyCreated() : std::invalid_argument("VirusAlreadyCreated") {};
};

struct VirusNotFound : public std::invalid_argument {
    VirusNotFound() : std::invalid_argument("VirusNotFound") {};
};

struct TriedToRemoveStemVirus : public std::invalid_argument {
    TriedToRemoveStemVirus() : std::invalid_argument("TriedToRemoveStemVirus") {};
};

template <typename Virus>
class VirusGenealogy {
private:
    struct VirusNode {
        Virus virus;
        std::set<std::shared_ptr<VirusNode>,
                std::owner_less<std::shared_ptr<VirusNode>>> children;
        std::set<std::weak_ptr<VirusNode>,
                std::owner_less<std::weak_ptr<VirusNode>>> parents;
        int parents_counter;

        VirusNode(typename Virus::id_type const &id) : virus(id) {
            parents_counter = 0;
        };
    };

    std::shared_ptr<VirusNode> stem_node;
    std::map<typename Virus::id_type, std::shared_ptr<VirusNode>> viruses;

    std::shared_ptr<VirusNode> get_node(typename Virus::id_type const &id) const {
        auto it = viruses.find(id);
        if (it == viruses.end())
            throw VirusNotFound{};
        return it->second;
    }

public:
    VirusGenealogy(typename Virus::id_type const &stem_id) {
        stem_node = std::make_shared<VirusNode>(stem_id);
        viruses[stem_id] = stem_node;
    }

    VirusGenealogy(VirusGenealogy const &) = delete;

    VirusGenealogy &operator=(VirusGenealogy const &) = delete;

    typename Virus::id_type get_stem_id() const {
        return stem_node->virus.get_id();
    }

    bool exists(typename Virus::id_type const &id) const {
        return viruses.contains(id);
    }

    const Virus& operator[](typename Virus::id_type const &id) const {
        return get_node(id)->virus;
    }

    void create(typename Virus::id_type const &id, std::vector<typename Virus::id_type> const &parents_ids) {
        if (parents_ids.empty())
            return;

        auto virus = std::make_shared<VirusNode>(id);
        for (auto const &p_id : parents_ids)
            virus->parents.insert(get_node(p_id));
        virus->parents_counter = virus->parents.size();

        auto [virus_it, inserted] = viruses.insert({id, virus});
        if (!inserted)
            throw VirusAlreadyCreated{};

        auto parents_it = virus->parents.begin();
        try {
            for (; parents_it != virus->parents.end(); ++parents_it)
                parents_it->lock()->children.insert(virus);
        } catch (...) {
            viruses.erase(virus_it);
            while (parents_it != virus->parents.begin())
                (--parents_it)->lock()->children.erase(virus);
            throw;
        }
        //std::cout << " " << virus->parents_counter  << " " << virus->parents.size() << "\n";
    }

    void create(typename Virus::id_type const &id, typename Virus::id_type const &parent_id) {
        /* Potem można się zastanowić, czy nie byłoby lepiej napisać oddzielną
        wersję. */
        create(id, std::vector<typename Virus::id_type>{parent_id});
    }

    std::vector<typename Virus::id_type> get_parents(typename Virus::id_type const &id) const {
        std::vector<typename Virus::id_type> parents_ids;
        for (auto p : get_node(id)->parents)
            parents_ids.push_back(p.lock()->virus.get_id());

        return parents_ids;
    }

    void connect(typename Virus::id_type const &child_id, typename Virus::id_type const &parent_id) {
        auto child_ptr = get_node(child_id);
        auto parent_ptr = get_node(parent_id);

        if (child_ptr->parents.insert(parent_ptr).second) {
            try {
                parent_ptr->children.insert(child_ptr);
                child_ptr->parents_counter++;
            } catch (...) {
                child_ptr->parents.erase(parent_ptr);
                throw;
            }
        }
    }

    void remove_dfs(typename Virus::id_type const &id,
                    std::vector<typename std::map<typename Virus::id_type,
                    std::shared_ptr<VirusNode>>::iterator> &nodes_to_delete) {

        auto node_it = viruses.find(id);

        for (auto child : (node_it->second)->children) {
            if (child.get()->parents_counter == 1) {
                nodes_to_delete.push_back(viruses.find(child.get()->virus.get_id()));
                remove_dfs(child.get()->virus.get_id(), nodes_to_delete);
            }
            else
                child.get()->parents_counter--;
        }
    }

    void remove(typename Virus::id_type const &id) {
        if (!viruses.contains(id)) {
            throw VirusNotFound{};
        } else if (get_stem_id() == id) {
            throw TriedToRemoveStemVirus{};
        }

        std::vector<typename std::map<typename Virus::id_type,
                std::shared_ptr<VirusNode>>::iterator> nodes_to_delete;
        try {
            auto node_it = viruses.find(id);
            nodes_to_delete.push_back(node_it);
            remove_dfs(id, nodes_to_delete);

            // usuwamy nas z dzieci naszych rodziców - dotyczy tylko pierwszego usuwanego wierzholka
            for (auto it = node_it->second->parents.begin(); it != node_it->second->parents.end(); ++it) {
                it->lock()->children.erase(node_it->second);
            }
        } catch (...) {
            // naprawiamy counterki
            for (auto node : nodes_to_delete) {
                node->second->parents_counter = node->second->parents.size();
            }
            throw;
        }

        // usuwamy dzieci bez rodzica
        for (auto node : nodes_to_delete) {
            //usuwamy siebie jako rodzica naszych dzieci
            for (auto child : (node->second)->children) {
                child.get()->parents.erase(node->second);
            }
            //usuwamy siebie
            viruses.erase(node);
        }
    }

    class children_iterator : public std::iterator<std::bidirectional_iterator_tag, Virus> {
    private:
        /* Takie określenie typu chyba nie jest za ładne, ew do poprawy. */
        using set_iterator_t = typename decltype(VirusNode::children)::iterator;

        set_iterator_t it;

    public:
        children_iterator() = default;

        explicit children_iterator(set_iterator_t it) : it(it) {};

        Virus const &operator*() const { return (*it)->virus; }

        Virus *operator->() const { return &(*it)->virus; }

        constexpr bool operator==(children_iterator const &other) const {
            return it == other.it;
        }

        children_iterator &operator++() { it++; return *this; }

        children_iterator operator++(int) { return children_iterator(it++); }

        children_iterator &operator--() { it--; return *this; }

        children_iterator operator--(int) { return children_iterator(it--); }
    };

    children_iterator get_children_begin(typename Virus::id_type const &id) const {
        return children_iterator(get_node(id)->children.begin());
    }

    children_iterator get_children_end(typename Virus::id_type const &id) const {
        return children_iterator(get_node(id)->children.end());
    }
};

//TODO BASIA: remove (strong)
//TODO MIESZKO: create, get_parents, connect, iterator 1, iterator 2,
//CIEKAWE:
//DONE: wyjąteczki

#endif // VIRUS_GENEALOGY_H
