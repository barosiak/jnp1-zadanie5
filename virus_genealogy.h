#ifndef JNP1_ZADANIE5_VIRUS_GENEALOGY_H
#define JNP1_ZADANIE5_VIRUS_GENEALOGY_H

#include <map>
#include <vector>
#include <memory>

// PYTANIE: czy te virtual są dobrze użyte?

struct VirusAlreadyCreated : public virtual std::exception {
    virtual const char *what() const noexcept {
        return "VirusAlreadyCreated";
    }
};

struct VirusNotFound : public virtual std::exception {
    virtual const char *what() const noexcept {
        return "VirusNotFound";
    }
};

struct TriedToRemoveStemVirus : public virtual std::exception {
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

private:
};

template <typename Virus>
class VirusGenealogy {
public:
    VirusGenealogy(typename Virus::id_type const &stem_id) {}

private:
    std::map<typename Virus::id_type, std::shared_ptr<VirusNode<Virus>>> viruses;
};

//TODO BASIA:
//TODO MIESZKO: 
//CIEKAWE: iterator 1, iterator 2, get_parents, create, connect, remove (strong)
//DONE: wyjąteczki

#endif //JNP1_ZADANIE5_VIRUS_GENEALOGY_H
