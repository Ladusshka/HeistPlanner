#ifndef __PROGTEST__
#include <cassert>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <functional>
#include <bitset>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <compare>

struct Vault {
    unsigned value;
    unsigned pebbles;
    std::vector<std::pair<unsigned, unsigned>> missing_connections;
};

struct UnlockingSequence {
    unsigned vault_id;
    std::vector<bool> moved_pebbles;
};

#endif







static bool build_moved_pebbles(const Vault& vault, std::vector<bool>& moved_pebbles_out) {
    const unsigned N = vault.pebbles;
    if (N % 2 != 0) return false;
    const unsigned target = N / 2;

    std::vector<std::vector<unsigned>> adj(N);
    for (auto [u, v] : vault.missing_connections) {
        if (u >= N || v >= N) continue;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    std::vector<int> group(N, -1);
    std::vector<std::vector<unsigned>> components;
    for (unsigned i = 0; i < N; ++i) {
        if (group[i] != -1) continue;
        int comp_id = (int)components.size();
        components.emplace_back();
        std::queue<unsigned> q;
        q.push(i);
        group[i] = comp_id;
        while (!q.empty()) {
            unsigned curr = q.front(); q.pop();
            components[comp_id].push_back(curr);
            for (unsigned next : adj[curr]) {
                if (group[next] == -1) {
                    group[next] = comp_id;
                    q.push(next);
                }
            }
        }
    }

    // Subset sum DP
    std::vector<int> dp_ss(target + 1, -1);
    dp_ss[0] = -2; // base case
    for (int i = 0; i < (int)components.size(); ++i) {
        unsigned weight = components[i].size();
        for (int w = target; w >= (int)weight; --w) {
            if (dp_ss[w] == -1 && dp_ss[w - weight] != -1) {
                dp_ss[w] = i;
            }
        }
    }

    if (dp_ss[target] == -1) return false;

    std::vector<bool> moved(N, false);
    int curr_w = target;
    while (curr_w > 0) {
        int comp_idx = dp_ss[curr_w];
        for (unsigned p : components[comp_idx]) moved[p] = true;
        curr_w -= components[comp_idx].size();
    }
    moved_pebbles_out = std::move(moved);
    return true;
}

std::vector<UnlockingSequence> plan_heist(
        const std::vector<Vault>& vaults,
        unsigned transition_time,
        unsigned max_time
) {
    struct Candidate {
        unsigned id;
        unsigned value;
        unsigned cost; // pebbles / 2
        std::vector<bool> config;
    };

    std::vector<Candidate> candidates;
    for (unsigned i = 0; i < (unsigned)vaults.size(); ++i) {
        std::vector<bool> config;
        if (build_moved_pebbles(vaults[i], config)) {
            candidates.push_back({i, vaults[i].value, vaults[i].pebbles / 2, std::move(config)});
        }
    }

    if (candidates.empty()) return {};

    // DP: dp[t] = max_value
    // transition_time se platí mezi trezory.
    // Pokud vybereme K trezorů, celkový čas = sum(unlock_times) + (K-1) * transition_time.
    // To lze přepsat jako: sum(unlock_times + transition_time) - transition_time.

    std::vector<long long> dp(max_time + 1, -1);
    std::vector<std::vector<int>> keep(candidates.size(), std::vector<int>(max_time + 1, 0));
    dp[0] = 0;

    for (int i = 0; i < (int)candidates.size(); ++i) {
        unsigned cost = candidates[i].cost;
        // Pro první trezor je cena jen 'cost', pro další 'cost + transition_time'
        // Ale jednodušší je počítat vše jako 'cost + transition_time' a pak na konci
        // přičíst transition_time k limitu max_time (pokud jsme vybrali aspoň jeden).
        unsigned effective_cost = cost + transition_time;

        for (int t = max_time; t >= (int)effective_cost; --t) {
            if (dp[t - effective_cost] != -1) {
                long long new_val = dp[t - effective_cost] + candidates[i].value;
                if (new_val > dp[t]) {
                    dp[t] = new_val;
                    keep[i][t] = 1;
                }
            }
        }
    }

    // Najdeme nejlepší výsledek. Musíme zohlednit, že u prvního trezoru
    // jsme "neoprávněně" přičetli transition_time.
    // Takže se díváme na indexy až do (max_time + transition_time).

    long long best_v = 0;
    int best_t = -1;

    // Upravený limit: pokud vybereme aspoň jeden, máme k dobru jeden transition_time


    // Potřebujeme mírně upravit DP, aby správně reflektovalo "první trezor".
    // Jednodušší trik: spustíme DP s tím, že každý trezor stojí (cost + trans).
    // Výsledek hledáme v dp[t] kde t <= max_time + transition_time.

    std::vector<long long> final_dp(max_time + transition_time + 1, -1);
    final_dp[0] = 0;
    std::vector<std::vector<char>> used(candidates.size(), std::vector<char>(max_time + transition_time + 1, 0));

    for (int i = 0; i < (int)candidates.size(); ++i) {
        int cost = (int)candidates[i].cost + (int)transition_time;
        for (int t = max_time + transition_time; t >= cost; --t) {
            if (final_dp[t - cost] != -1) {
                if (final_dp[t - cost] + candidates[i].value > final_dp[t]) {
                    final_dp[t] = final_dp[t - cost] + candidates[i].value;
                    used[i][t] = 1;
                }
            }
        }
    }

    for (int t = 0; t <= (int)max_time + (int)transition_time; ++t) {
        if (final_dp[t] > best_v) {
            best_v = final_dp[t];
            best_t = t;
        }
    }

    if (best_t <= 0) return {};

    std::vector<UnlockingSequence> result;
    int curr_t = best_t;
    for (int i = (int)candidates.size() - 1; i >= 0; --i) {
        if (used[i][curr_t]) {
            result.push_back({candidates[i].id, candidates[i].config});
            curr_t -= (candidates[i].cost + transition_time);
        }
    }

    return result;
}

#ifndef __PROGTEST__

struct TestFailed : std::runtime_error {
    using std::runtime_error::runtime_error;
};

#define CHECK(cond, msg) do { \
    if (!(cond)) throw TestFailed(msg); \
  } while (0)

void check_unlocking_sequence(
        unsigned pebbles,
        const std::vector<std::pair<unsigned, unsigned>>& missing_connections,
        const std::vector<bool>& moved
) {
    CHECK(moved.size() == pebbles, "Solution has wrong size.\n");

    size_t moved_cnt = 0;
    for (bool p : moved) moved_cnt += p;
    CHECK(2*moved_cnt == pebbles,
          "Exactly half of the pebbles must be moved.\n");

    for (auto [ u, v ] : missing_connections) CHECK(moved[u] == moved[v],
                                                    "Pebble not connected with all on other side.\n");
}

void check_solution(
        const std::vector<UnlockingSequence>& solution,
        unsigned expected_value,
        const std::vector<Vault>& vaults,
        unsigned transition_time,
        unsigned max_time
) {
    unsigned time = 0, value = 0;
    std::vector<bool> robbed(vaults.size(), false);

    for (size_t i = 0; i < solution.size(); i++) {
        const auto& [ id, moved ] = solution[i];

        CHECK(id < vaults.size(), "Id is out of range.\n");
        CHECK(!robbed[id], "Robbed same vault twice.\n");
        robbed[id] = true;

        const auto& vault = vaults[id];
        value += vault.value;

        if (i != 0) time += transition_time;
        time += vault.pebbles / 2;
        CHECK(time <= max_time, "Run out of time.\n");

        check_unlocking_sequence(vault.pebbles, vault.missing_connections, moved);
    }

    CHECK(value == expected_value, "Total value mismatch.\n");
}


struct Test {
    unsigned expected_value;
    unsigned max_time;
    unsigned transition_time;
    std::vector<Vault> vaults;
};

inline const std::vector<Test> TESTS = {
        Test{
                .expected_value = 3010, .max_time = 3, .transition_time = 8,
                .vaults = {
                        { .value = 3010, .pebbles = 6, .missing_connections = { {3,4}, {0,1}, {4,5}, {5,3}, } },
                        { .value = 3072, .pebbles = 6, .missing_connections = { {2,1}, {1,3}, {0,1}, {0,3}, {4,5}, {2,3}, } },
                        { .value = 5069, .pebbles = 10, .missing_connections = { {7,2}, {3,4}, {0,1}, {8,4}, {1,2}, {8,3}, {7,0}, {5,6}, {9,5}, {9,6}, } },
                        { .value = 2061, .pebbles = 4, .missing_connections = { {3,0}, {2,1}, {0,2}, {1,3}, } },
                }
        },
        Test{
                .expected_value = 6208, .max_time = 13, .transition_time = 12,
                .vaults = {
                        { .value = 6011, .pebbles = 12, .missing_connections = { {1,5}, {2,4}, {5,10}, {1,10}, {0,3}, {8,3}, {8,0}, {9,8}, {2,6}, {3,9}, {0,9}, {4,6}, {11,7}, } },
                        { .value = 2056, .pebbles = 4, .missing_connections = { {1,0}, {2,0}, {2,1}, } },
                        { .value = 5885, .pebbles = 12, .missing_connections = { {1,6}, {3,7}, {1,0}, {2,9}, {9,8}, {2,8}, {5,7}, {11,4}, {10,1}, {5,3}, {0,10}, } },
                        { .value = 5818, .pebbles = 12, .missing_connections = { {9,0}, {7,1}, {6,4}, {8,6}, {4,2}, {11,5}, {5,3}, {9,7}, {8,4}, {2,8}, {10,11}, {5,10}, {10,3}, {9,1}, } },
                        { .value = 4880, .pebbles = 10, .missing_connections = { {7,3}, {4,1}, {9,2}, {6,9}, {2,6}, {5,0}, {8,4}, } },
                        { .value = 5233, .pebbles = 10, .missing_connections = { {0,2}, {4,5}, {8,3}, {9,7}, {7,1}, {6,3}, {6,8}, } },
                        { .value = 6208, .pebbles = 12, .missing_connections = { {1,7}, {3,4}, {10,7}, {0,3}, {8,2}, {5,1}, {9,11}, {0,6}, {6,3}, {10,1}, {0,4}, } },
                        { .value = 4182, .pebbles = 8, .missing_connections = { {5,7}, {7,4}, {4,5}, {1,0}, {5,6}, {3,1}, {6,4}, {0,3}, {6,7}, } },
                }
        },
        Test{
                .expected_value = 1, .max_time = 100, .transition_time = 8,
                .vaults = {
                        { .value = 1, .pebbles = 14, .missing_connections = {
                                {0,1}, {0,2}, {3,4}, {4,5}, {6,7}, {8,9}, {10,11}, {12,13},
                        } },
                }
        },
};

int main() {
    int ok = 0, fail = 0;

    for (auto t : TESTS) {
        try {
            auto sol = plan_heist(t.vaults, t.transition_time, t.max_time);
            check_solution(sol, t.expected_value, t.vaults, t.transition_time, t.max_time);
            ok++;
        } catch (const TestFailed&) {
            fail++;
        }
    }

    if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
    else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
}

#endif


