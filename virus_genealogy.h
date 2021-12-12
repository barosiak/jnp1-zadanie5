#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <map>
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
    std::vector<std::weak_ptr<VirusNode<Virus>>> parents;

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
        try {
            return viruses.at(id)->virus;
        } catch(const std::out_of_range&) {
            /* PYTANIE: Czy tak jest ok? Nasze metody mają rzucać dalej wszystkie
            wyjątki od wirusów, czy można założyć, że wirus nie rzuci 
            std::out_of_range, które my weźmiemy za rzucone przez mapę? */
            throw VirusNotFound{};
        }
    }

private:
    std::shared_ptr<VirusNode<Virus>> stem_node;
    std::map<typename Virus::id_type, std::shared_ptr<VirusNode<Virus>>> viruses;
};

//TODO BASIA:
//TODO MIESZKO: create
//CIEKAWE: iterator 1, iterator 2, get_parents, connect, remove (strong)
//DONE: wyjąteczki

#endif // VIRUS_GENEALOGY_H
