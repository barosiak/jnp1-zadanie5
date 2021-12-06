#ifndef JNP1_ZADANIE5_VIRUS_GENEALOGY_H
#define JNP1_ZADANIE5_VIRUS_GENEALOGY_H

#include <map>
#include <vector>
#include <memory>

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
//DONE:

#endif //JNP1_ZADANIE5_VIRUS_GENEALOGY_H
