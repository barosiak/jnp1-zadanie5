#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <map>
#include <vector>
#include <memory>
#include <stdexcept>

// PYTANIE: czy te virtual są dobrze użyte?

struct VirusAlreadyCreated : public virtual std::invalid_argument {
    virtual const char *what() const noexcept {
        return "VirusAlreadyCreated";
    }
};

struct VirusNotFound : public virtual std::invalid_argument {
    virtual const char *what() const noexcept {
        return "VirusNotFound";
    }
};

struct TriedToRemoveStemVirus : public virtual std::invalid_argument {
    virtual const char *what() const noexcept {
        return "TriedToRemoveStemVirus";
    }
};

//to bym ukryła
template <typename Virus>
class VirusNode {
public:
    Virus virus;
    std::vector<std::shared_ptr<VirusNode<Virus>>> children;
    std::vector<std::weak_ptr<VirusNode<Virus>>> parents;

    VirusNode(typename Virus::id_type const &id) : virus(id) {};
private:
};

template <typename Virus>
class VirusGenealogy {
public:
    VirusGenealogy(typename Virus::id_type const &stem_id) {
        stem_node = std::make_shared<VirusNode<Virus>>(stem_id);
        /* Jeżeli poniżej poleci wyjątek, to będzie ok, bo pamięć zaalokowana
        na stem_node się zwolni automatycznie dzięki użyciu shared_ptr 
        (sprawdzałem!). */
        viruses[stem_id] = stem_node;
    }

    Virus::id_type get_stem_id() const {
        /* nie zmieniamy stanu naszej klasy, więc żaden wyjątek nam nie
        zaszkodzi */
        return stem_node->virus.get_id();
    }

private:
    std::shared_ptr<VirusNode<Virus>> stem_node;
    std::map<typename Virus::id_type, std::shared_ptr<VirusNode<Virus>>> viruses;
};

//TODO BASIA:
//TODO MIESZKO: 
//CIEKAWE: iterator 1, iterator 2, get_parents, create, connect, remove (strong)
//DONE: wyjąteczki

#endif // VIRUS_GENEALOGY_H
