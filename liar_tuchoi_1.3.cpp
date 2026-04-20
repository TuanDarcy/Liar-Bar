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

        if (targetCount + jCount > 0) {
            numPlay = 1; // AI đánh an toàn 1 lá
            bool found = false;
            for (int i = 0; i < (int)hand.size(); i++) {
                if (hand[i].type == requiredType) {
                    chosenIndices.push_back(i);
                    found = true; break;
                }
            }
            if (!found) {
                for (int i = 0; i < (int)hand.size(); i++) {
                    if (hand[i].type == JOKER) {
                        chosenIndices.push_back(i);
                        break;
                    }
                }
            }
        } else {
            numPlay = 1;
            chosenIndices.push_back(0);
        }
    }
};

class Game {
public:
    int bulletPos;
    int currentChamber;
    CardType requiredType; 
    vector<Card> deck;
    vector<Player> players;

    Game() {
        srand((unsigned int)time(0));
        players.push_back(Player(0, true));
        players.push_back(Player(1, false));
        players.push_back(Player(2, false));
        players.push_back(Player(3, false));
        resetGun();
        prepareDeck();
    }

    // CẬP NHẬT: Tỉ lệ 1/3 (3 ổ đạn)
    void resetGun() {
        bulletPos = rand() % 3;
        currentChamber = 0;
    }

    void announceRequiredType() {
        int r = rand() % 3;
        requiredType = (r == 0) ? Q : (r == 1 ? K : ACE);
        string name = (requiredType == Q) ? "Q" : (requiredType == K ? "K" : "A");
        cout << MAGENTA << "\n[QUẢN TRÒ]: Đổi yêu cầu! Vòng này phải đánh lá " << BOLD << name << RESET << endl;
    }

    void prepareDeck() {
        deck.clear();
        for (int i = 0; i < 6; i++) deck.push_back({Q});
        for (int i = 0; i < 6; i++) deck.push_back({K});
        for (int i = 0; i < 6; i++) deck.push_back({ACE});
        for (int i = 0; i < 2; i++) deck.push_back({JOKER});
    }

    // Trả về true nếu người bị bắn thiệt mạng
    bool fireGun(int targetId) {
        cout << RED << ">> Đưa súng lên đầu Player " << targetId << " và bóp cò..." << RESET << endl;
        SLEEP_MS(1500);

        bool killed = (currentChamber == bulletPos);
        if (killed) {
            cout << RED << BOLD << " BÙM!!! (Player " << targetId << " ĐÃ THIỆT MẠNG)" << RESET << "\n";
            players[targetId].alive = false;
            resetGun();
        } else {
            cout << GREEN << " *Cạch* (Đạn không nổ. May mắn sống sót!)" << RESET << "\n";
            // CẬP NHẬT: Xoay trong 3 ổ đạn
            currentChamber = (currentChamber + 1) % 3;
        }

        // YÊU CẦU 1: Quản trò đổi lá bài sau mỗi lần bóp cò
        announceRequiredType();
        return killed;
    }

    void dealCards() {
        random_shuffle(deck.begin(), deck.end());
        int dIdx = 0;
        for (auto &p : players) {
            p.hand.clear();
            p.alive = true;
            for (int i = 0; i < 5; i++) p.hand.push_back(deck[dIdx++]);
        }
    }

    void play() {
        SET_UTF8();
        dealCards();
        cout << MAGENTA << "==========================================\n";
        cout << "   LIAR ROULETTE: CHIẾN THẮNG SINH TỬ\n";
        cout << "   (Tỉ lệ tử vong: 1/3)\n";
        cout << "==========================================\n" << RESET;

        announceRequiredType();

        int turn = 0;
        while (true) {
            int currentId = turn % 4;
            Player &current = players[currentId];

            if (!current.alive) {
                turn++; continue;
            }

            cout << "\n------------------------------------------\n";
            cout << YELLOW << "LƯỢT CỦA PLAYER " << currentId << (current.isHuman ? " (BẠN)" : " (AI)") << RESET << " (Còn " << current.hand.size() << " lá)\n";

            int nextId = (currentId + 1) % 4;
            while (!players[nextId].alive) {
                nextId = (nextId + 1) % 4;
            }

            int n = 0;
            vector<int> chosenIndices;
            int actualCorrect = 0;

            if (current.isHuman) {
                current.showHand();
                cout << "=> Quản trò yêu cầu: " << BOLD << (requiredType == Q ? "Q" : (requiredType == K ? "K" : "A")) << RESET << endl;
                cout << "=> Đánh mấy lá? (1-3): "; cin >> n;
                while(n < 1 || n > 3 || n > (int)current.hand.size()) {
                    cout << "Không hợp lệ! Nhập lại: "; cin >> n;
                }
                for (int i = 0; i < n; i++) {
                    int idx;
                    cout << "Chọn lá thứ " << i + 1 << ": "; cin >> idx;
                    chosenIndices.push_back(idx - 1);
                }
            } else {
                current.aiDecideMove(requiredType, n, chosenIndices);
                SLEEP_MS(1000);
            }

            for (int idx : chosenIndices) {
                CardType t = current.hand[idx].type;
                if (t == requiredType || t == JOKER) actualCorrect++;
            }

            bool isTruth = (actualCorrect == n);
            string reqName = (requiredType == Q) ? "Q" : (requiredType == K ? "K" : "A");
            cout << "Player " << currentId << " úp " << n << " lá và hô: " << CYAN << n << " lá " << reqName << RESET << endl;

            bool someoneDoubts = false;
            // YÊU CẦU 2: Người tiếp theo check khi người chơi đánh hết bài
            if (players[nextId].isHuman) {
                cout << "Nghi ngờ? (1: Có / 0: Tin): "; int choice; cin >> choice;
                someoneDoubts = (choice == 1);
            } else {
                // AI luôn check nếu đối thủ đánh lá cuối cùng để về đích
                if (current.hand.size() == (size_t)n) someoneDoubts = true;
                else someoneDoubts = (rand() % 100 < 35); // AI trở nên đa nghi hơn với tỉ lệ 1/3
                
                if (someoneDoubts) cout << MAGENTA << "Player " << nextId << " hô: \"NÓI DỐI!\"\n" << RESET;
            }

            SLEEP_MS(800);

            bool playerWon = false;
            if (someoneDoubts) {
                if (isTruth) {
                    cout << GREEN << "=> NÓI THẬT! " << RESET;
                    fireGun(nextId);
                    // Nếu nói thật ở lá cuối cùng -> Thắng luôn
                    if (current.hand.size() == (size_t)n) playerWon = true;
                } else {
                    cout << RED << "=> NÓI DỐI! " << RESET;
                    bool died = fireGun(currentId);
                    // YÊU CẦU 2: Nếu nói dối lá cuối mà SỐNG -> Thắng, CHẾT -> Loại
                    if (current.hand.size() == (size_t)n) {
                        if (!died) playerWon = true;
                        else cout << RED << "Player " << currentId << " đã chết ngay trước cổng thiên đường!" << RESET << endl;
                    }
                }
            } else if (current.hand.size() == (size_t)n) {
                // Thắng âm thầm nếu không ai check
                playerWon = true;
            }

            if (playerWon) {
                cout << GREEN << BOLD << "\n🎉 PLAYER " << currentId << " ĐÃ CHIẾN THẮNG TRÒ CHƠI! 🎉\n" << RESET;
                return;
            }

            current.removeCardsByIndices(chosenIndices);

            int aliveCount = 0, lastId = -1;
            for (auto &p : players) { if (p.alive) { aliveCount++; lastId = p.id; } }
            if (aliveCount <= 1) {
                cout << GREEN << BOLD << "\n🏆 PLAYER " << lastId << " LÀ NGƯỜI SỐNG SÓT CUỐI CÙNG! 🏆\n" << RESET;
                return;
            }

            turn++;
            SLEEP_MS(1000);
        }
    }
};

int main() {
    Game game;
    game.play();
    return 0;
}