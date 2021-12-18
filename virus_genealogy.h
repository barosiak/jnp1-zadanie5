#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <map>
#include <set>
#include <vector>
#include <memory>
#include <stdexcept>

struct VirusAlreadyCreated : public std::invalid_argument {
    VirusAlreadyCreated() : std::invalid_argument("VirusAlreadyCreated") {};
};

struct VirusNotFound : public std::invalid_argument {
    VirusNotFound() : std::invalid_argument("VirusNotFound") {};
};

struct TriedToRemoveStemVirus : public std::invalid_argument {
    TriedToRemoveStemVirus() : std::invalid_argument("TriedToRemoveStemVirus") {};
};

// Class representing a graph of virus genealogy.
template<typename Virus>
class VirusGenealogy {
private:
    // Structure representing a virus node in the graph.
    struct VirusNode {
        Virus virus;
        std::set<std::shared_ptr<VirusNode>,
                 std::owner_less<std::shared_ptr<VirusNode>>> children;
        std::set<std::weak_ptr<VirusNode>,
                 std::owner_less<std::weak_ptr<VirusNode>>> parents;
        int parents_counter = 0;

        VirusNode(typename Virus::id_type const &id) : virus(id) {};
    };

    std::shared_ptr<VirusNode> stem_node;
    std::map<typename Virus::id_type, std::shared_ptr<VirusNode>> viruses;

    // Helper function returning a node of a virus with given id.
    std::shared_ptr<VirusNode> get_node(typename Virus::id_type const &id) const {
        auto it = viruses.find(id);

        if (it == viruses.end())
            throw VirusNotFound{};

        return it->second;
    }

    // Helper function, adds to a passed vector iterators of elements to remove.
    void remove_dfs(typename std::shared_ptr<VirusNode> node_it,
                    std::vector<typename std::map<typename Virus::id_type,
                    std::shared_ptr<VirusNode>>::iterator> &nodes_to_delete) {

        for (auto child: (node_it)->children) {
            if (child->parents_counter == 1) {
                nodes_to_delete.push_back(viruses.find(child->virus.get_id()));
                remove_dfs(child, nodes_to_delete);
            } else {
                child->parents_counter--;
            }
        }
    }

public:
    VirusGenealogy(typename Virus::id_type const &stem_id) {
        stem_node = std::make_shared<VirusNode>(stem_id);
        viruses[stem_id] = stem_node;
    }

    VirusGenealogy(VirusGenealogy const &) = delete;

    VirusGenealogy &operator=(VirusGenealogy const &) = delete;

    // Returns the id of the stem virus.
    typename Virus::id_type get_stem_id() const {
        return stem_node->virus.get_id();
    }

    // Checks if a virus with given id exists.
    bool exists(typename Virus::id_type const &id) const {
        return viruses.contains(id);
    }

    // Returns a reference to a virus with given id if it exists.
    const Virus &operator[](typename Virus::id_type const &id) const {
        return get_node(id)->virus;
    }

    // Creates a virus with given id and parents if possible.
    void create(typename Virus::id_type const &id,
                std::vector<typename Virus::id_type> const &parents_ids) {
        if (parents_ids.empty())
            return;

        auto virus = std::make_shared<VirusNode>(id);
        for (auto const &parent_id: parents_ids)
            virus->parents.insert(get_node(parent_id));

        virus->parents_counter = virus->parents.size();

        auto[virus_it, inserted] = viruses.insert({id, virus});
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
    }

    // Creates a virus with given id and parent if possible.
    void create(typename Virus::id_type const &id,
                typename Virus::id_type const &parent_id) {
        create(id, std::vector<typename Virus::id_type>{parent_id});
    }

    // Returns a vector of parents of virus with given id if possible.
    std::vector<typename Virus::id_type>
    get_parents(typename Virus::id_type const &id) const {
        std::vector<typename Virus::id_type> parents_ids;

        for (auto parent: get_node(id)->parents)
            parents_ids.push_back(parent.lock()->virus.get_id());

        return parents_ids;
    }

    // Connects given viruses if possible.
    void connect(typename Virus::id_type const &child_id,
                 typename Virus::id_type const &parent_id) {

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

    // Removes a virus with given id from the genealogy if possible.
    void remove(typename Virus::id_type const &id) {
        if (!viruses.contains(id))
            throw VirusNotFound{};
        else if (get_stem_id() == id)
            throw TriedToRemoveStemVirus{};

        std::vector<typename std::map<typename Virus::id_type,
                    std::shared_ptr<VirusNode>>::iterator> nodes_to_delete;

        try {
            auto node = viruses.find(id)->second;
            nodes_to_delete.push_back(viruses.find(id));
            remove_dfs(node, nodes_to_delete);

            for (auto it = node->parents.begin(); it != node->parents.end(); ++it)
                it->lock()->children.erase(node);

        } catch (...) {
            for (auto node: nodes_to_delete)
                for (auto ch: node->second->children)
                    ch->parents_counter = ch->parents.size();

            throw;
        }

        for (auto node: nodes_to_delete) {
            for (auto child: (node->second)->children)
                child->parents.erase(node->second);

            viruses.erase(node);
        }
    }

    // Iterator class for children of nodes in virus genealogy.
    class children_iterator
        : public std::iterator<std::bidirectional_iterator_tag, Virus> {
    private:
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

        children_iterator &operator++() {
            it++;
            return *this;
        }

        children_iterator operator++(int) { return children_iterator(it++); }

        children_iterator &operator--() {
            it--;
            return *this;
        }

        children_iterator operator--(int) { return children_iterator(it--); }
    };

    // Returns an iterator to the beginning of children vector
    // of a node with given id.
    children_iterator get_children_begin(typename Virus::id_type const &id) const {
        return children_iterator(get_node(id)->children.begin());
    }

    // Returns an iterator to the end of children vector of a node with given id.
    children_iterator get_children_end(typename Virus::id_type const &id) const {
        return children_iterator(get_node(id)->children.end());
    }
};

#endif // VIRUS_GENEALOGY_H
