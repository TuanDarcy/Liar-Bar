#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>
#include <iomanip>

// Cấu hình hiển thị tiếng Việt
#ifdef _WIN32
#include <windows.h>
#define SET_UTF8() SetConsoleOutputCP(65001)
#else
#include <unistd.h>
#define SET_UTF8()
#endif

using namespace std;

// --- Định nghĩa màu sắc Console ---
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BOLD    "\033[1m"

enum CardType { Q, K, ACE, JOKER };

struct Card {
    CardType type;
};

class Player {
public:
    int id;
    vector<Card> hand;
    bool alive = true;
    int strategy; // 0: Thật thà, 1: Liều lĩnh, 2: Tối ưu (Master)

    // Thống kê mô phỏng
    int wins = 0;
    int survivalCount = 0;

    Player(int _id, int _strat) : id(_id), strategy(_strat) {}

    int countType(CardType t) {
        int count = 0;
        for (auto &c : hand) if (c.type == t) count++;
        return count;
    }

    void removeCardsByIndices(vector<int> indices) {
        sort(indices.rbegin(), indices.rend());
        for (int idx : indices) {
            if (idx >= 0 && idx < (int)hand.size()) {
                hand.erase(hand.begin() + idx);
            }
        }
    }

    // AI THUẬT TOÁN ĐÁNH BÀI (MÔ PHỎNG CHIẾN THUẬT TÂM LÝ)
    void aiDecideMove(CardType requiredType, int &numPlay, vector<int> &chosenIndices) {
        chosenIndices.clear();
        int targetCount = countType(requiredType);
        int jCount = countType(JOKER);
        int handSize = (int)hand.size();

        if (strategy == 2) {
            // === 1. TỐI ƯU (The Mastermind) ===

            // Chốt hạ: Gộp toàn bộ Joker + Bài thật để về đích an toàn
            if (targetCount + jCount >= handSize && handSize <= 3) {
                numPlay = handSize;
                for(int i=0; i<(int)hand.size(); i++) if(hand[i].type == requiredType) chosenIndices.push_back(i);
                for(int i=0; i<(int)hand.size() && (int)chosenIndices.size() < numPlay; i++) if(hand[i].type == JOKER) chosenIndices.push_back(i);
                return;
            }

            // Giăng bẫy: Đánh 3 lá thật -> Dụ đối thủ "Check" để tự sát
            if (targetCount >= 3) {
                numPlay = 3;
                for(int i=0; i<(int)hand.size() && (int)chosenIndices.size() < 3; i++) {
                    if(hand[i].type == requiredType) chosenIndices.push_back(i);
                }
                return;
            }

            // Xả rác: Đầu game (còn 5 lá) úp 1 lá giả -> Rủi ro bị nghi ngờ thấp
            if (handSize == 5 && targetCount < 3) {
                numPlay = 1;
                for(int i=0; i<(int)hand.size(); i++) {
                    if(hand[i].type != requiredType && hand[i].type != JOKER) {
                        chosenIndices.push_back(i);
                        break;
                    }
                }
                if (!chosenIndices.empty()) return;
            }

            // Tiết kiệm: Giữ Joker cho giai đoạn sinh tử, đánh bài thật lẻ trước
            if (targetCount > 0) {
                numPlay = 1;
                for(int i=0; i<(int)hand.size(); i++) if(hand[i].type == requiredType) { chosenIndices.push_back(i); break; }
            } else if (jCount > 0 && handSize <= 2) { // Chỉ dùng Joker khi sắp hết bài
                numPlay = 1;
                for(int i=0; i<(int)hand.size(); i++) if(hand[i].type == JOKER) { chosenIndices.push_back(i); break; }
            } else {
                numPlay = 1; chosenIndices.push_back(0); // Buộc phải đánh 1 lá giả
            }

        } else if (strategy == 1) {
            // === 2. LIỀU LĨNH (The Aggressive) ===
            // Hành vi: Luôn đánh Max 3 lá mọi lượt (trộn lẫn Thật & Giả)
            numPlay = min(3, handSize);

            // Ưu tiên bài thật nếu có
            for(int i=0; i<(int)hand.size() && (int)chosenIndices.size() < numPlay; i++) {
                if(hand[i].type == requiredType || hand[i].type == JOKER) chosenIndices.push_back(i);
            }
            // Lấp đầy bằng bài giả cho đủ 3 lá (Gây áp lực cao)
            for(int i=0; i<(int)hand.size() && (int)chosenIndices.size() < numPlay; i++) {
                bool alreadyChosen = false;
                for(int idx : chosenIndices) if(idx == i) alreadyChosen = true;
                if(!alreadyChosen) chosenIndices.push_back(i);
            }
        } else {
            // === 3. THẬT THÀ (The Honest) ===
            // Hành vi: "Có sao đánh vậy" (Chỉ ra bài Thật + Joker)
            numPlay = min(3, targetCount + jCount);

            if (numPlay > 0) {
                for(int i=0; i<(int)hand.size() && (int)chosenIndices.size() < numPlay; i++) {
                    if(hand[i].type == requiredType || hand[i].type == JOKER) chosenIndices.push_back(i);
                }
            } else {
                // Bị ép buộc: Chỉ úp 1 lá giả khi bài trên tay hoàn toàn vô dụng
                numPlay = 1;
                chosenIndices.push_back(0);
            }
        }

        if (chosenIndices.empty()) { numPlay = 1; chosenIndices.push_back(0); }
    }

    // AI THUẬT TOÁN NGHI NGỜ (AUTO PLAY)
    bool aiDoubt(int n, CardType declaredType, bool isLastCard) {
        if (isLastCard) return true; // Luôn check lá cuối cùng

        float doubtChance = 0.0f;
        if (strategy == 2) { // Mastermind: Phân tích rủi ro
            doubtChance = (n == 3) ? 0.85f : (n == 2 ? 0.40f : 0.12f);
            // Nếu mình đang cầm nhiều lá đó thì đối thủ khả năng cao đang nói dối
            doubtChance += (countType(declaredType) * 0.18f);
        } else if (strategy == 1) { // Aggressive: Thích lật kèo
            doubtChance = 0.55f; // Nghi ngờ cao mặc định
        } else { // Honest: Dè dặt, ít nghi ngờ
            doubtChance = (n == 3) ? 0.45f : (n == 2 ? 0.15f : 0.05f);
        }

        if (doubtChance > 1.0f) doubtChance = 1.0f;
        return ((rand() % 100) / 100.0f < doubtChance);
    }
};

class Simulation {
public:
    int bulletPos, currentChamber;
    CardType requiredType;
    vector<Card> deck;
    vector<Player> players;

    Simulation() {
        srand((unsigned int)time(0));
        players.push_back(Player(0, 2)); // Mastermind
        players.push_back(Player(1, 1)); // Aggressive
        players.push_back(Player(2, 0)); // Honest
        players.push_back(Player(3, 2)); // Mastermind
    }

    void resetGun() { bulletPos = rand() % 3; currentChamber = 0; }
    void announceRequiredType() { requiredType = (CardType)(rand() % 3); }

    void prepareDeck() {
        deck.clear();
        for (int i = 0; i < 6; i++) { deck.push_back({Q}); deck.push_back({K}); deck.push_back({ACE}); }
        deck.push_back({JOKER}); deck.push_back({JOKER});
    }

    void run(int numMatches) {
        SET_UTF8();
        cout << CYAN << "Bắt đầu mô phỏng " << numMatches << " trận đấu với 3 chiến thuật tâm lý...\n" << RESET;

        for (int m = 0; m < numMatches; m++) {
            prepareDeck();
            random_shuffle(deck.begin(), deck.end());
            int dIdx = 0;
            for (auto &p : players) {
                p.hand.clear(); p.alive = true;
                for (int i = 0; i < 5; i++) p.hand.push_back(deck[dIdx++]);
            }
            resetGun();
            announceRequiredType();

            int turn = 0;
            while (true) {
                int currId = turn % 4;
                if (!players[currId].alive) { turn++; continue; }

                Player &curr = players[currId];
                int nextId = (currId + 1) % 4;
                while (!players[nextId].alive) nextId = (nextId + 1) % 4;

                int n = 0;
                vector<int> chosenIndices;
                curr.aiDecideMove(requiredType, n, chosenIndices);

                int correct = 0;
                for (int i : chosenIndices) if (curr.hand[i].type == requiredType || curr.hand[i].type == JOKER) correct++;
                bool isTruth = (correct == (int)chosenIndices.size());

                bool doubt = players[nextId].aiDoubt(n, requiredType, curr.hand.size() == (size_t)n);

                bool matchOver = false;
                if (doubt) {
                    if (isTruth) {
                        if (currentChamber == bulletPos) { players[nextId].alive = false; resetGun(); }
                        else currentChamber = (currentChamber + 1) % 3;
                        announceRequiredType();
                        if (curr.hand.size() == (size_t)n) { curr.wins++; matchOver = true; }
                    } else {
                        if (currentChamber == bulletPos) { players[currId].alive = false; resetGun(); }
                        else {
                            currentChamber = (currentChamber + 1) % 3;
                            if (curr.hand.size() == (size_t)n) { curr.wins++; matchOver = true; }
                        }
                        announceRequiredType();
                    }
                } else if (curr.hand.size() == (size_t)n) {
                    curr.wins++; matchOver = true;
                }

                if (matchOver) break;
                curr.removeCardsByIndices(chosenIndices);

                int aliveCount = 0;
                int survivorId = -1;
                for (auto &p : players) if (p.alive) { aliveCount++; survivorId = p.id; }
                if (aliveCount <= 1) { if(survivorId != -1) players[survivorId].wins++; break; }

                turn++;
            }
            for (auto &p : players) if (p.alive) p.survivalCount++;
        }

        cout << MAGENTA << "===============================================================\n" << RESET;
        cout << YELLOW << "           KẾT QUẢ MÔ PHỎNG " << numMatches << " TRẬN ĐẤU\n" << RESET;
        cout << MAGENTA << "===============================================================\n" << RESET;
        cout << left << setw(30) << "Chiến thuật" << right << setw(15) << "Tỉ lệ Thắng" << setw(15) << "Tỉ lệ Sống" << endl;
        cout << "---------------------------------------------------------------\n";

        string strats[] = {"Thật thà (Honest)", "Liều lĩnh (Aggressive)", "Tối ưu (Mastermind)"};
        for (auto &p : players) {
            string label = "P" + to_string(p.id) + " (" + strats[p.strategy] + ")";
            float winRate = (float)p.wins / numMatches * 100;
            float survivalRate = (float)p.survivalCount / numMatches * 100;

            cout << left << setw(30) << label
                 << fixed << setprecision(2) << right << setw(12) << winRate << " %"
                 << setw(13) << survivalRate << " %" << endl;
        }
        cout << "---------------------------------------------------------------\n";
        cout << GREEN << "Lưu ý:" << RESET << " Tỉ lệ thắng của Mastermind thường cao nhất nhờ khả năng bẫy đối thủ.\n";
    }
};

int main() {
    SET_UTF8();
    Simulation sim;
    sim.run(100000);
    return 0;
}
