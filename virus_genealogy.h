#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

//TYMCZASOWO!!
#include <iostream>

#include <map>
#include <set>
#include <vector>
#include <memory>
#include <stdexcept>

// PYTANIE: czy te virtual są dobrze użyte?

struct VirusAlreadyCreated : public virtual std::invalid_argument {
    VirusAlreadyCreated() : std::invalid_argument("VirusAlreadyCreated") {};
};

struct VirusNotFound : public virtual std::invalid_argument {
    VirusNotFound() : std::invalid_argument("VirusNotFound") {};
};

struct TriedToRemoveStemVirus : public virtual std::invalid_argument {
    TriedToRemoveStemVirus() : std::invalid_argument("TriedToRemoveStemVirus") {};
};

//to bym ukryła
template <typename Virus>
class VirusNode {
public:
    Virus virus;
    std::vector<std::shared_ptr<VirusNode<Virus>>> children;
    std::set<typename Virus::id_type> parents;

    VirusNode(Virus::id_type const &id) : virus(id) {};
private:
};

template <typename Virus>
class VirusGenealogy {
public:
    VirusGenealogy(Virus::id_type const &stem_id) {
        stem_node = std::make_shared<VirusNode<Virus>>(stem_id);
        /* Jeżeli poniżej poleci wyjątek, to będzie ok, bo pamięć zaalokowana
        na stem_node się zwolni automatycznie dzięki użyciu shared_ptr 
        (sprawdzałem!). */
        viruses[stem_id] = stem_node;
    }

    /* oba poniższe to wymóg treści zadania */
    VirusGenealogy(VirusGenealogy const &) = delete;

    VirusGenealogy &operator=(VirusGenealogy const &) = delete;

    Virus::id_type get_stem_id() const {
        /* nie zmieniamy stanu naszej klasy, więc żaden wyjątek nam nie
        zaszkodzi */
        return stem_node->virus.get_id();
    }

    bool exists(Virus::id_type const &id) const {
        /* nie zmieniamy stanu naszej klasy, więc żaden wyjątek nam nie
        zaszkodzi */
        return viruses.contains(id);
    }

    const Virus& operator[](Virus::id_type const &id) const {
        return get_node(id)->virus;
    }

    void create(Virus::id_type const &id, std::vector<typename Virus::id_type> const &parents_ids) {
        if (viruses.contains(id))
            throw VirusAlreadyCreated{};
        
        auto virus = std::make_shared<VirusNode<Virus>>(id);
        virus->parents = std::set<typename Virus::id_type>(parents_ids.begin(), parents_ids.end());
        std::vector<std::shared_ptr<VirusNode<Virus>>> parents_pointers;

        for (auto const &p_id : virus->parents)
            parents_pointers.push_back(get_node(p_id));

        /* Do tego miejsca nie wprowadziliśmy jeszcze żadnej zmiany w strukturze.
        Jeżeli poniżej poleci wyjątek, to nadal nie będzie żadnej zmiany. */
        viruses[id] = virus;

        /* Od tej pory już żaden wyjątek zdarzyć się nie może... */
        for (auto const &p : parents_pointers)
            p->children.push_back(virus);
    }

    void create(Virus::id_type const &id, Virus::id_type const &parent_id) {
        /* Potem można się zastanowić, czy nie byłoby lepiej napisać oddzielną
        wersję. */
        create(id, std::vector<typename Virus::id_type>{parent_id});
    }

    std::vector<typename Virus::id_type> get_parents(Virus::id_type const &id) const {
        /* wyjątki nam nie straszne, bo nic nie zmieniamy */
        auto const &parents = get_node(id)->parents;
        return {parents.begin(), parents.end()};
    }

    void connect(Virus::id_type const &child_id, Virus::id_type const &parent_id) {
        auto child_ptr = get_node(child_id);
        auto parent_ptr = get_node(parent_id);

        /* Do tej pory nie wprowadziliśmy żadnej zmiany. Przy wstawianiu do seta
        może polecieć wyjątek, ale wtedy nadal nie będzie żadnej zmiany. Po
        wstawieniu do seta jesteśmy już bezpieczni, wstawienie do vectora
        nie może spowodować wyjątku. */
        if (child_ptr->parents.insert(parent_id).second)
            /* Jeżeli tej krawędzi wcześniej nie było... */
            parent_ptr->children.push_back(child_ptr);
    }

private:
    std::shared_ptr<VirusNode<Virus>> stem_node;
    std::map<typename Virus::id_type, std::shared_ptr<VirusNode<Virus>>> viruses;

    std::shared_ptr<VirusNode<Virus>> get_node(Virus::id_type const &id) const {
        try {
            return viruses.at(id);
        } catch(const std::out_of_range&) {
            /* PYTANIE: Czy tak jest ok? Nasze metody mają rzucać dalej wszystkie
            wyjątki od wirusów, czy można założyć, że wirus nie rzuci 
            std::out_of_range, które my weźmiemy za rzucone przez mapę? */
            throw VirusNotFound{};
        }
    }
};

//TODO BASIA:
//TODO MIESZKO: create, get_parents, connect, 
//CIEKAWE: iterator 1, iterator 2, remove (strong)
//DONE: wyjąteczki

#endif // VIRUS_GENEALOGY_H
