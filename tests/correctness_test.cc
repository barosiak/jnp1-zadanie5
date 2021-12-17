#include <bits/stdc++.h>
#include "../virus_genealogy.h"

using namespace std;

class Virus {
public:
	using id_type = int;

	Virus(Virus::id_type _id)
		: id(_id)
	{}

	Virus::id_type get_id() const {
		return id;
	}

private:
	id_type id;
};

template<class Ex>
bool check_throws_up(auto f) {
	try {
		f();
	}
	catch(const Ex &e) {
		return true;
	}
	catch(...) {
		assert(false);
	}
	assert(false);
	return false;
}

template<class T> bool has_same_content(std::vector<T> a, std::vector<T> b) {
	sort(a.begin(), a.end());
	sort(b.begin(), b.end());
	return a == b;
}

using Iter = VirusGenealogy<Virus>::children_iterator;

bool has_same_addresses(Iter begin, Iter end, std::vector<const Virus*> addresses) {
	std::vector<const Virus*> v;
	for(; begin != end; ++begin)
		v.emplace_back(&(*begin));
	return has_same_content(v, addresses);
}

const int MAX_N = 100;
const int OPS = 100000;
int root = 0;
VirusGenealogy<Virus> G(root);
set<int> ch[MAX_N];
set<int> p[MAX_N];
mt19937 rng(0);

int rand(int a, int b) {
    return uniform_int_distribution<int>(a, b)(rng);
}

// jego rodzicom usunelismy juz dzieci
void remove_dfs(int w) {
    p[w].clear();
    for (int c : ch[w]) {
        if (p[c].size() == 1)
            remove_dfs(c);
        else
            p[c].erase(w);
    }
    ch[w].clear();
}

int main() {
    check_throws_up<TriedToRemoveStemVirus>([&] {G.remove(root); });
    for (int op = 0; op < OPS; op++) {
        vector<int> free_nums;
        vector<int> occup_nums;
        for (int i = 0; i < MAX_N; i++) {
            if (i != 0 && p[i].empty())
                free_nums.push_back(i);
            else
                occup_nums.push_back(i);
        }

        int type = rand(0, 5);
        if (type == 0) { // ok create
            if (free_nums.empty())
                continue;

            int id = free_nums[rand(0, free_nums.size() - 1)];
            while(occup_nums.back() > id)
                occup_nums.pop_back();
            shuffle(occup_nums.begin(), occup_nums.end(), rng);

            /* USUNAC -1!! */
            int to_remove = rand(0, occup_nums.size() - 1);
            while (to_remove--)
                occup_nums.pop_back();

            cout << op << "/" << OPS << ": create(" << id << ", {";
            for (int i = 0 ; i < occup_nums.size() ; i++) {
                cout << occup_nums[i];
                if (i + 1 < occup_nums.size())
                    cout << ", ";
            }
            cout << "});\n";

            G.create(id, occup_nums);

            for (int w : occup_nums) {
                ch[w].insert(id);
                p[id].insert(w);
            }
        } else if (type == 1) { // ok remove
            if (occup_nums.size() == 1)
                continue;

            int id = occup_nums[rand(1, occup_nums.size() - 1)];
            cout << op << "/" << OPS << ": remove(" << id << ");\n";

            G.remove(id);

            for (int w : p[id])
                ch[w].erase(id);
            remove_dfs(id);
        } else if (type == 2) { // ok connect
            if (occup_nums.size() == 1)
                continue;

            int a_ind = rand(0, occup_nums.size() - 2);
            int a = occup_nums[a_ind];
            int b = occup_nums[rand(a_ind + 1, occup_nums.size() - 1)];

            cout << op << "/" << OPS << ": connect(" << b << ", " << a << ");\n";
            G.connect(b, a);

            ch[a].insert(b);
            p[b].insert(a);
        } else if (type == 3) { // remove nonexistent
            if (free_nums.empty())
                continue;

            int id = free_nums[rand(0, free_nums.size() - 1)];
            cout << op << "/" << OPS << ": remove(" << id << ");\n";

            check_throws_up<VirusNotFound>([&] {G.remove(id);});
        } else if (type == 4) { // connect parent nonexistent
            if (free_nums.size() == 0)
                continue;

            int c = occup_nums[rand(0, occup_nums.size() - 1)];
            int pp = free_nums[rand(0, free_nums.size() - 1)];
            cout << op << "/" << OPS << ": connect(" << c << ", " << pp << ");\n";

            check_throws_up<VirusNotFound>([&] {G.connect(c, pp);});
        } else if (type == 5) { // connect child nonexistent
            if (free_nums.size() == 0)
                continue;

            int pp = occup_nums[rand(0, occup_nums.size() - 1)];
            int c = free_nums[rand(0, free_nums.size() - 1)];
            cout << op << "/" << OPS << ": connect(" << c << ", " << pp << ");\n";

            check_throws_up<VirusNotFound>([&] {G.connect(c, pp);});
        }

        assert(G.get_stem_id() == root);
        for (int w = 0 ; w < MAX_N ; w++) {
            if (w != 0 && p[w].empty())
                assert(!G.exists(w));
            else {
                assert(G.exists(w));
                assert(has_same_content(G.get_parents(w),
                    std::vector<int>(p[w].begin(), p[w].end())));
                vector<const Virus*> addresses;
                for (int u : ch[w])
                    addresses.push_back(&G[u]);
                assert(has_same_addresses(G.get_children_begin(w),
                    G.get_children_end(w), addresses));
            }
        }
    }    
    return 0;
}