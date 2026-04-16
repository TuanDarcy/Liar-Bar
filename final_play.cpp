#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>
#include <iomanip>

// Khai báo thư viện Sleep tương thích đa nền tảng
#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(x) Sleep(x)
#else
#include <unistd.h>
#define SLEEP_MS(x) usleep((x)*1000)
#endif

using namespace std;

// --- Định nghĩa màu sắc Console ---
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"

enum CardType { Q, K, ACE, JOKER };

struct Card {
    CardType type;
    string name() {
        switch (type) {
            case Q: return "Q";
            case K: return YELLOW "K" RESET;
            case ACE: return "A";
            case JOKER: return RED "J" RESET;
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
        cout << CYAN << "Bai tren tay ban: " << RESET;
        for (size_t i = 0; i < hand.size(); i++) {
            cout << "(" << i + 1 << ")[" << hand[i].name() << "] ";
        }
        cout << " (Tong: " << hand.size() << " la)\n";
    }

    // Xóa bài dựa trên index (chính xác tuyệt đối)
    void removeCardsByIndices(vector<int> indices) {
        // Sắp xếp index giảm dần để khi xóa không bị thay đổi thứ tự các lá phía sau
        sort(indices.rbegin(), indices.rend());
        for (int idx : indices) {
            if (idx >= 0 && idx < hand.size()) {
                hand.erase(hand.begin() + idx);
            }
        }
    }

    // AI Quyết định tổng quát cho mọi loại bài (Q, K, A)
    void aiDecideMove(int &numPlay, CardType &declaredType, vector<int> &chosenIndices) {
        chosenIndices.clear();
        int qCount = countType(Q);
        int kCount = countType(K);
        int aCount = countType(ACE);
        int jCount = countType(JOKER);

        // Tìm loại bài có số lượng nhiều nhất (ưu tiên để làm "Bài Thật")
        int maxCount = max({qCount, kCount, aCount});
        if (maxCount == qCount) declaredType = Q;
        else if (maxCount == kCount) declaredType = K;
        else declaredType = ACE;

        if (maxCount + jCount > 0) {
            // Có bài -> Quyết định đánh Thật
            numPlay = min(3, maxCount + jCount);

            // Ưu tiên chọn bài thật trước
            for (int i = 0; i < hand.size() && chosenIndices.size() < numPlay; i++) {
                if (hand[i].type == declaredType) chosenIndices.push_back(i);
            }
            // Dùng Joker bù vào số lượng còn thiếu
            for (int i = 0; i < hand.size() && chosenIndices.size() < numPlay; i++) {
                if (hand[i].type == JOKER) chosenIndices.push_back(i);
            }
        } else {
            // Trường hợp cực hiếm: Toàn Joker hoặc buộc phải đánh Dối
            declaredType = Q; // Hô bừa 1 loại
            numPlay = 1;
            chosenIndices.push_back(0); // Bỏ đi 1 lá đầu tiên để giấu
        }
    }
};

class Game {
public:
    int bulletPos;
    int currentChamber;
    vector<Card> deck;
    vector<Player> players;

    Game() {
        srand(time(0));
        players.push_back(Player(0, true));  // Bạn
        players.push_back(Player(1, false)); // AI 1
        players.push_back(Player(2, false)); // AI 2
        players.push_back(Player(3, false)); // AI 3
        resetGun();
    }

    void resetGun() {
        bulletPos = rand() % 6; // Vị trí viên đạn ngẫu nhiên (1/6)
        currentChamber = 0;
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

    // Hàm bắn súng với xác suất 1/6 (Russian Roulette)
    void fireGun(int targetId) {
        cout << RED << ">> Dua sung len dau Player " << targetId << " va bop co..." << RESET;
        SLEEP_MS(1500);

        if (currentChamber == bulletPos) {
            cout << RED << " BUM!!! (Player " << targetId << " DA CHET)" << RESET << "\n";
            players[targetId].alive = false;
            resetGun(); // Nạp đạn mới sau khi nổ
        } else {
            cout << GREEN << " *Cach* (Dan khong no. May man song sot!)" << RESET << "\n";
            currentChamber = (currentChamber + 1) % 6;
        }
    }

    void play() {
        prepareDeck();
        dealCards();
        cout << MAGENTA << "==========================================\n";
        cout << "   CHAO MUNG DEN VOI ROULETTE CARD GAME\n";
        cout << "   (Luật Gốc: Bắn đạn 1/6 - Russian Roulette)\n";
        cout << "==========================================\n" << RESET;

        int turn = 0;
        while (true) {
            int currentId = turn % 4;
            Player &current = players[currentId];

            if (!current.alive || current.hand.empty()) {
                turn++; continue;
            }

            cout << "\n------------------------------------------\n";
            cout << YELLOW << "LUOT CUA PLAYER " << currentId << (current.isHuman ? " (BAN)" : " (AI)") << RESET << "\n";

            int nextId = (currentId + 1) % 4;
            while (!players[nextId].alive || players[nextId].hand.empty()) {
                nextId = (nextId + 1) % 4;
            }

            int n = 0;
            CardType declaredType;
            vector<int> chosenIndices;
            int actualTypeCount = 0, actualJokerCount = 0, fakeCount = 0;

            if (current.isHuman) {
                current.showHand();

                // 1. Cho phép người chơi tự chọn loại bài muốn Hô
                cout << "=> Ban muon HO loai bai nao? (0: Q, 1: K, 2: A): ";
                int typeChoice;
                cin >> typeChoice;
                while(typeChoice < 0 || typeChoice > 2) { cout << "Nhap lai (0/1/2): "; cin >> typeChoice; }
                declaredType = (typeChoice == 0) ? Q : (typeChoice == 1 ? K : ACE);

                // 2. Chọn số lượng
                cout << "=> Ban muon danh may la? (1-3): ";
                cin >> n;
                while(n < 1 || n > 3 || n > (int)current.hand.size()) {
                    cout << "So luong khong hop le! Nhap lai: "; cin >> n;
                }

                // 3. Chỉ định chính xác vị trí bài muốn rút
                for (int i = 0; i < n; i++) {
                    int idx;
                    cout << "Chon la bai thu " << i + 1 << " (nhap vi tri 1-" << current.hand.size() << "): ";
                    cin >> idx;

                    while (idx < 1 || idx > (int)current.hand.size() || find(chosenIndices.begin(), chosenIndices.end(), idx - 1) != chosenIndices.end()) {
                        cout << RED << "Vi tri khong hop le hoac la bai nay da duoc chon! Nhap lai: " << RESET;
                        cin >> idx;
                    }
                    chosenIndices.push_back(idx - 1);
                }
            }
            else {
                // Phân quyền chọn loại bài cho AI
                current.aiDecideMove(n, declaredType, chosenIndices);
                SLEEP_MS(1000);
            }

            // Đếm số lượng thực tế trong các lá bài ĐÃ ĐƯỢC RÚT
            for (int idx : chosenIndices) {
                CardType t = current.hand[idx].type;
                if (t == declaredType) actualTypeCount++;
                else if (t == JOKER) actualJokerCount++;
                else fakeCount++;
            }

            // Xử lý tên loại bài để in ra
            string declaredName = (declaredType == Q) ? "Q" : (declaredType == K ? YELLOW "K" RESET : "A");

            if (current.isHuman) {
                cout << "\nBan up " << n << " la bai xuong ban va ho to: " << CYAN << "\"" << n << " la " << declaredName << "!\"\n" << RESET;
            } else {
                cout << "Player " << currentId << " up " << n << " la bai xuong ban va ho: " << CYAN << "\"" << n << " la " << declaredName << "!\"\n" << RESET;
            }

            // Công thức Thật/Dối TỔNG QUÁT: Số lá đúng loại + Joker == Số lá hô
            bool isTruth = (actualTypeCount + actualJokerCount == n);
            bool someoneDoubts = false;

            // Nếu người kế tiếp là Human VÀ còn sống thì mới hỏi ý kiến
            if (players[nextId].isHuman) {
                cout << "\nPlayer " << currentId << " ho la " << declaredName << ". Ban co nghi ngo ho dang NOI DOI khong?\n";
                cout << "(1 = Co bieu hien luon leo, nghi ngo ngay! / 0 = Tin tuong bo qua): ";
                int choice; cin >> choice;
                someoneDoubts = (choice == 1);
            } else {
                // TÍCH HỢP BẢNG XÁC SUẤT VÀ TÂM LÝ TỪ LƯU ĐỒ CỦA BẠN
                float doubtChance = 0.0f;

                // Mức độ nghi ngờ cơ bản dựa trên số lá bài đánh ra (n)
                if (n == 3) {
                    doubtChance = 0.85f; // Cực cao (~11.74% có bài thật -> Vùng nguy hiểm)
                } else if (n == 2) {
                    doubtChance = 0.40f; // Trung bình (~35.22% có bài thật)
                } else if (n == 1) {
                    doubtChance = 0.15f; // Thấp (~38.74% có bài thật -> Không đáng liều mạng để check)
                }

                // Cộng dồn tâm lý: Người chơi càng ít bài, đối thủ càng dễ Check
                // (100% - tỉ lệ sở hữu bài như bạn ghi trong readme)
                float handSizeFactor = (5.0f - current.hand.size()) * 0.1f;
                doubtChance += handSizeFactor;

                // Random quyết định dựa trên Doubt Chance
                someoneDoubts = ((rand() % 100) / 100.0f < doubtChance);

                if (someoneDoubts) {
                    cout << MAGENTA << "Player " << nextId << " ho to: \"DOI TRA! TOI NGHI NGO (Ti le check: " << (int)(doubtChance*100) << "%)!\"\n" << RESET;
                } else {
                    cout << "Player " << nextId << " tin tuong bo qua (Ti le check: " << (int)(doubtChance*100) << "%).\n";
                }
            }
            SLEEP_MS(1000);

            if (someoneDoubts) {
                cout << "Lat bai cua Player " << currentId << ": Co " << actualTypeCount << " " << declaredName << ", " << actualJokerCount << " Joker, va " << fakeCount << " la khac.\n";
                if (isTruth) {
                    cout << GREEN << "=> Player " << currentId << " NOI THAT!\n" << RESET;
                    cout << "Nguoi nghi ngo (Player " << nextId << ") phai chiu phat!\n";
                    fireGun(nextId);
                } else {
                    cout << RED << "=> Player " << currentId << " NOI DOI!\n" << RESET;
                    cout << "Ke noi doi (Player " << currentId << ") phai chiu phat!\n";
                    fireGun(currentId);
                }
            }

            // Loại bỏ các lá bài dựa trên vị trí đã đánh
            current.removeCardsByIndices(chosenIndices);

            int aliveCount = 0;
            int winnerId = -1;
            for (auto &p : players) {
                if (p.alive && p.hand.empty()) {
                    cout << GREEN << "\n🎉 Player " << p.id << " DA HET BAI VA GIANH CHIEN THANG! 🎉\n" << RESET;
                    return;
                }
                if (p.alive) { aliveCount++; winnerId = p.id; }
            }

            if (aliveCount <= 1) {
                cout << GREEN << "\n🏆 Player " << winnerId << " LA NGUOI SONG SOT CUOI CUNG! 🏆\n" << RESET;
                return;
            }

            // Auto-play delay - Không cần nhấn Enter
            if (!players[0].alive && !players[currentId].isHuman) {
                cout << "\n[Ban da chet. AI dang tiep tuc mo phong chien dau...]" << endl;
            }
            SLEEP_MS(2500);
            turn++;
        }
    }
};

int main() {
    Game game;
    game.play();
    return 0;
}
