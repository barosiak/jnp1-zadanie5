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

struct VirusAlreadyCreated : public std::invalid_argument {
    VirusAlreadyCreated() : std::invalid_argument("VirusAlreadyCreated") {};
};

struct VirusNotFound : public std::invalid_argument {
    VirusNotFound() : std::invalid_argument("VirusNotFound") {};
};

struct TriedToRemoveStemVirus : public std::invalid_argument {
    TriedToRemoveStemVirus() : std::invalid_argument("TriedToRemoveStemVirus") {};
};

//to bym ukryła
template <typename Virus>
class VirusNode {
public:
    Virus virus;
    /* Vectory to bardzo słabe pomysły, bo chcemy szybko usuwać... */
    std::set<std::shared_ptr<VirusNode<Virus>>,
             std::owner_less<std::shared_ptr<VirusNode<Virus>>>> children;
    std::set<std::weak_ptr<VirusNode<Virus>>,
             std::owner_less<std::weak_ptr<VirusNode<Virus>>>> parents;

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
        for (auto const &p_id : parents_ids)
            virus->parents.insert(get_node(p_id));

        /* Do tego miejsca nie wprowadziliśmy jeszcze żadnej zmiany w strukturze.
        Jeżeli poniżej poleci wyjątek, to nadal nie będzie żadnej zmiany. */
        viruses[id] = virus;

        /* Od tej pory już żaden wyjątek zdarzyć się nie może... */
        /* MOŻE!!!!!!!!!!!!!!!!!!!!!! */
        // PYTANIE: czy auto, czy auto const &? auto
        for (auto p : virus->parents)
            p.lock()->children.insert(virus);
    }

    void create(Virus::id_type const &id, Virus::id_type const &parent_id) {
        /* Potem można się zastanowić, czy nie byłoby lepiej napisać oddzielną
        wersję. */
        create(id, std::vector<typename Virus::id_type>{parent_id});
    }

    std::vector<typename Virus::id_type> get_parents(Virus::id_type const &id) const {
        /* wyjątki nam nie straszne, bo nic nie zmieniamy */

        std::vector<typename Virus::id_type> parents_ids;
        // PYTANIE: czy auto, czy auto const & ???
        for (auto p : get_node(id)->parents)
            parents_ids.push_back(p.lock()->virus.get_id());

        return parents_ids;
    }

    void connect(Virus::id_type const &child_id, Virus::id_type const &parent_id) {
        auto child_ptr = get_node(child_id);
        auto parent_ptr = get_node(parent_id);

        /* Do tej pory nie wprowadziliśmy żadnej zmiany. A niżej nie poleci
        już żaden wyjątek. */
        child_ptr->parents.insert(parent_ptr);
        parent_ptr->children.insert(child_ptr);
    }

    // DZIEDZICZENIE PO ITERATOR
    class children_iterator {
    private:
        /* Takie określenie typu chyba nie jest za ładne, ew do poprawy. */
        using set_iterator_t = decltype(VirusNode<Virus>::children)::iterator;

        set_iterator_t it; 

    public:
        using iterator_type = set_iterator_t;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Virus;
        using difference_type = set_iterator_t::difference_type;
        using reference = Virus const &;
        using pointer = Virus *; // PYTANIE: const czy nie??

        children_iterator() = default;

        explicit children_iterator(iterator_type it) : it(it) {};

        reference operator*() const { return (*it)->virus; }

        pointer operator->() const { return &(*it)->virus; }

        bool operator==(children_iterator const &) const = default;

        children_iterator &operator++() { it++; return *this; }

        children_iterator operator++(int) { return children_iterator(it++); }

        children_iterator &operator--() { it--; return *this; }

        children_iterator operator--(int) { return children_iterator(it--); }

        // PYTANIE: nie trzeba implementować +, +=, -, -=? NIE
    };

    children_iterator get_children_begin(Virus::id_type const &id) const {
        return children_iterator(get_node(id)->children.begin());
    }

    children_iterator get_children_end(Virus::id_type const &id) const {
        return children_iterator(get_node(id)->children.end());
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
            /* NIE, NIE MOŻEMY */
            throw VirusNotFound{};
        }
    }
};

//TODO BASIA: remove (strong)
//TODO MIESZKO: create, get_parents, connect, iterator 1, iterator 2, 
//CIEKAWE: 
//DONE: wyjąteczki

#endif // VIRUS_GENEALOGY_H
