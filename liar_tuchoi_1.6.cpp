#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>
#include <iomanip>

// Cấu hình hiển thị tiếng Việt và Sleep đa nền tảng
#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(x) Sleep(x)
#define SET_UTF8() SetConsoleOutputCP(65001)
#else
#include <unistd.h>
#define SLEEP_MS(x) usleep((x)*1000)
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
    string name() {
        switch (type) {
            case Q: return "Q";
            case K: return YELLOW "K" RESET;
            case ACE: return "A";
            case JOKER: return RED "Joker" RESET;
            default: return "";
        }
    }
};

class Player {
public:
    int id;
    vector<Card> hand;
    bool alive = true;
    bool isHuman;

    Player(int _id, bool _isHuman) : id(_id), isHuman(_isHuman) {}

    int countType(CardType t) {
        int count = 0;
        for (auto &c : hand) if (c.type == t) count++;
        return count;
    }

    void showHand() {
        cout << CYAN << "Bài trên tay bạn: " << RESET;
        for (size_t i = 0; i < hand.size(); i++) {
            cout << "(" << i + 1 << ")[" << hand[i].name() << "] ";
        }
        cout << " (Tổng: " << hand.size() << " lá)\n";
    }

    void removeCardsByIndices(vector<int> indices) {
        sort(indices.rbegin(), indices.rend());
        for (int idx : indices) {
            if (idx >= 0 && idx < (int)hand.size()) {
                hand.erase(hand.begin() + idx);
            }
        }
    }

    // AI chọn bài dựa trên loại mà Quản trò yêu cầu
    void aiDecideMove(CardType requiredType, int &numPlay, vector<int> &chosenIndices) {
        chosenIndices.clear();
        int targetCount = countType(requiredType);
        int jCount = countType(JOKER);

        // Chiến thuật AI: Ưu tiên đánh thật nếu có bài
        if (targetCount + jCount > 0) {
            numPlay = (rand() % min(3, targetCount + jCount)) + 1;
            for (int i = 0; i < (int)hand.size() && (int)chosenIndices.size() < numPlay; i++) {
                if (hand[i].type == requiredType) chosenIndices.push_back(i);
            }
            for (int i = 0; i < (int)hand.size() && (int)chosenIndices.size() < numPlay; i++) {
                if (hand[i].type == JOKER) chosenIndices.push_back(i);
            }
        } else {
            // Không có bài -> Đành nói dối 1 lá bài rác
            numPlay = 1;
            chosenIndices.push_back(0);
        }
    }
};

class Game {
public:
    int bulletPos;
    int currentChamber;
    CardType requiredType; // Loại bài Quản trò yêu cầu
    vector<Card> deck;
    vector<Player> players;

    Game() {
        srand(time(0));
        players.push_back(Player(0, true));
        players.push_back(Player(1, false));
        players.push_back(Player(2, false));
        players.push_back(Player(3, false));
        resetGun();
    }

    void resetGun() {
        bulletPos = rand() % 6;
        currentChamber = 0;
    }

    void announceRequiredType() {
        int r = rand() % 3;
        requiredType = (r == 0) ? Q : (r == 1 ? K : ACE);
        string name = (requiredType == Q) ? "Q" : (requiredType == K ? "K" : "A");
        cout << MAGENTA << "\n[QUẢN TRÒ]: Vòng này tất cả phải đánh lá " << BOLD << name << RESET << MAGENTA << "!" << RESET << endl;
    }

    void prepareDeck() {
        deck.clear();
        for (int i = 0; i < 6; i++) deck.push_back({Q});
        for (int i = 0; i < 6; i++) deck.push_back({K});
        for (int i = 0; i < 6; i++) deck.push_back({ACE});
        for (int i = 0; i < 2; i++) deck.push_back({JOKER});
        random_shuffle(deck.begin(), deck.end());
    }

    void dealCards() {
        int dIdx = 0;
        for (auto &p : players) {
            p.hand.clear();
            p.alive = true;
            for (int i = 0; i < 5; i++) p.hand.push_back(deck[dIdx++]);
        }
    }

    void fireGun(int targetId) {
        cout << RED << ">> Đưa súng lên đầu Player " << targetId << " và bóp cò..." << RESET;
        SLEEP_MS(1500);

        if (currentChamber == bulletPos) {
            cout << RED << BOLD << " BÙM!!! (Player " << targetId << " ĐÃ THIỆT MẠNG)" << RESET << "\n";
            players[targetId].alive = false;
            resetGun();
            // Sau khi có người chết, Quản trò đổi loại bài mới
            announceRequiredType();
        } else {
            cout << GREEN << " *Cạch* (Đạn không nổ. May mắn sống sót!)" << RESET << "\n";
            currentChamber = (currentChamber + 1) % 6;
        }
    }

    void play() {
        SET_UTF8();
        prepareDeck();
        dealCards();
        cout << MAGENTA << "==========================================\n";
        cout << "   CHÀO MỪNG ĐẾN VỚI ROULETTE CARD GAME\n";
        cout << "   (Hệ thống Quản trò chỉ định loại bài)\n";
        cout << "==========================================\n" << RESET;

        announceRequiredType();

        int turn = 0;
        while (true) {
            int currentId = turn % 4;
            Player &current = players[currentId];

            if (!current.alive || current.hand.empty()) {
                turn++; continue;
            }

            // Mỗi khi quay lại người chơi đầu tiên còn sống, Quản trò có thể đổi bài (tùy chọn)
            // Ở đây giữ nguyên bài cho đến khi có người bị bắn để tăng tính chiến thuật

            cout << "\n------------------------------------------\n";
            cout << YELLOW << "LƯỢT CỦA PLAYER " << currentId << (current.isHuman ? " (BẠN)" : " (AI)") << RESET << "\n";

            int nextId = (currentId + 1) % 4;
            while (!players[nextId].alive) {
                nextId = (nextId + 1) % 4;
            }

            int n = 0;
            vector<int> chosenIndices;
            int actualTypeCount = 0, actualJokerCount = 0, fakeCount = 0;

            if (current.isHuman) {
                current.showHand();
                string reqName = (requiredType == Q) ? "Q" : (requiredType == K ? "K" : "A");
                cout << "=> Quản trò yêu cầu: " << BOLD << reqName << RESET << endl;
                cout << "=> Bạn muốn đánh mấy lá? (1-3): ";
                cin >> n;
                while(n < 1 || n > 3 || n > (int)current.hand.size()) {
                    cout << "Số lượng không hợp lệ! Nhập lại: "; cin >> n;
                }

                for (int i = 0; i < n; i++) {
                    int idx;
                    cout << "Chọn lá bài thứ " << i + 1 << " (vị trí 1-" << current.hand.size() << "): ";
                    cin >> idx;
                    while (idx < 1 || idx > (int)current.hand.size() || find(chosenIndices.begin(), chosenIndices.end(), idx - 1) != chosenIndices.end()) {
                        cout << RED << "Vị trí không hợp lệ! Nhập lại: " << RESET;
                        cin >> idx;
                    }
                    chosenIndices.push_back(idx - 1);
                }
            } else {
                current.aiDecideMove(requiredType, n, chosenIndices);
                SLEEP_MS(1000);
            }

            for (int idx : chosenIndices) {
                CardType t = current.hand[idx].type;
                if (t == requiredType) actualTypeCount++;
                else if (t == JOKER) actualJokerCount++;
                else fakeCount++;
            }

            string reqName = (requiredType == Q) ? "Q" : (requiredType == K ? YELLOW "K" RESET : "A");
            cout << "\nPlayer " << currentId << " úp " << n << " lá bài và hô: " << CYAN << "\"" << n << " lá " << reqName << "!\"\n" << RESET;

            bool isTruth = (actualTypeCount + actualJokerCount == n);
            bool someoneDoubts = false;

            if (players[nextId].isHuman) {
                cout << "\nBạn có nghi ngờ họ NÓI DỐI không? (1: Có / 0: Tin): ";
                int choice; cin >> choice;
                someoneDoubts = (choice == 1);
            } else {
                float doubtChance = (n == 3) ? 0.75f : (n == 2 ? 0.35f : 0.10f);
                float handSizeFactor = (5.0f - current.hand.size()) * 0.12f;
                doubtChance += handSizeFactor;
                someoneDoubts = ((rand() % 100) / 100.0f < doubtChance);

                if (someoneDoubts) 
                    cout << MAGENTA << "Player " << nextId << " hô: \"NÓI DỐI! Tôi kiểm tra!\"\n" << RESET;
                else 
                    cout << "Player " << nextId << " tin tưởng bỏ qua.\n";
            }

            SLEEP_MS(800);

            if (someoneDoubts) {
                cout << "Lật bài: " << actualTypeCount << " " << reqName << ", " << actualJokerCount << " Joker, " << fakeCount << " lá khác.\n";
                if (isTruth) {
                    cout << GREEN << "=> NÓI THẬT! ";
                    fireGun(nextId);
                } else {
                    cout << RED << "=> NÓI DỐI! ";
                    fireGun(currentId);
                }
            }

            current.removeCardsByIndices(chosenIndices);

            // Kiểm tra điều kiện thắng
            if (current.alive && current.hand.empty()) {
                cout << GREEN << BOLD << "\n🎉 PLAYER " << currentId << " ĐÃ HẾT BÀI VÀ CHIẾN THẮNG! 🎉\n" << RESET;
                return;
            }

            int aliveCount = 0, lastId = -1;
            for (auto &p : players) { if (p.alive) { aliveCount++; lastId = p.id; } }
            if (aliveCount <= 1) {
                cout << GREEN << BOLD << "\n🏆 PLAYER " << lastId << " LÀ NGƯỜI SỐNG SÓT CUỐI CÙNG! 🏆\n" << RESET;
                return;
            }

            SLEEP_MS(1500);
            turn++;
        }
    }
};

int main() {
    Game game;
    game.play();
    return 0;
}