#include <gameutils.h>
#include <minimax.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <json.hpp>
#include <regex>
#include <sstream>
using json = nlohmann::json;

FullMove *processInput(std::string &input);

int column2Int(char c);

int row2Int(char r);

// 1=BLACK, 2=WHITE, 3=NEUTRON, 4=CELL
static constexpr uint8_t B = 1;
static constexpr uint8_t W = 2;
static constexpr uint8_t N = 3;
static constexpr uint8_t C = 4;

// Tablero inicial 5x5 en col-major: board[col*5 + row]
// r0: B B B B B
// r1: C C C C C
// r2: C C N C C
// r3: C C C C C
// r4: W W W W W
std::array<uint8_t, 25> initial_col_major() {
    std::array<uint8_t, 25> a{};
    // col 0
    a[0] = B;
    a[1] = C;
    a[2] = C;
    a[3] = C;
    a[4] = W;
    // col 1
    a[5] = B;
    a[6] = C;
    a[7] = C;
    a[8] = C;
    a[9] = W;
    // col 2
    a[10] = B;
    a[11] = C;
    a[12] = N;
    a[13] = C;
    a[14] = W;
    // col 3
    a[15] = B;
    a[16] = C;
    a[17] = C;
    a[18] = C;
    a[19] = W;
    // col 4
    a[20] = B;
    a[21] = C;
    a[22] = C;
    a[23] = C;
    a[24] = W;
    return a;
}

// void updateBoardWithMoves(std::array<uint8_t, 25> &boardArr, const std::vector<std::Tuple<>> &moves) {
//     for (const auto &move : moves) {
//         const int index = move->col * 5 + move->row;
//         boardArr[index] = static_cast<uint8_t>(move->kind);
//     }
// }

int main(int argc, char *argv[]) {
    try {
        constexpr int depth = 3;  // medium
        constexpr int ALPHA = std::numeric_limits<int>::min();
        constexpr int BETA = std::numeric_limits<int>::max();

        const auto boardArr = initial_col_major();

        std::cerr << "[boot] building Board, depth=" << depth << "\n";
        // Board board(boardArr);

        std::cerr << "[boot] calling maxValue...\n";
        auto board = std::make_unique<Board>(boardArr);
        const auto fm = maxValue(board, depth, ALPHA, BETA, PieceKind::BLACK);
        if (!fm) {
            std::cerr << "[error] no_fullmove_generated\n";
            return 2;
        }

        std::cout << "score: " << fm->score << "\n";
        if (!fm->moves.empty()) {
            const auto &mv = fm->moves;
            std::cout << "moves: " << mv.size() << "\n";
            for (size_t i = 0; i < mv.size(); ++i) {
                std::cout << "  #" << i << " row=" << mv[i]->row << " col=" << mv[i]->col << " kind=" << static_cast<int>(mv[i]->kind) << "\n";
            }
        } else {
            std::cout << "moves: 0\n";
        }

        return 0;
    } catch (const std::exception &ex) {
        std::cerr << "[exception] " << ex.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "[exception] unknown\n";
        return 1;
    }

    // --------------------------------------------------
    // if (argc > 1) {
    //     std::cout << "Reading...: " << argv[1] << std::endl;
    //     std::ifstream i(argv[1]);
    //     json savedGame;
    //     i >> savedGame;

    //     // auto board = boardJsonObj["board"].get<std::vector<std::vector<int>>>();
    //     auto movements = savedGame["movements"].get<std::vector<FullMove>>();

    //     std::cout << movements.size() << std::endl;
    // }
    // ---------------------------
    // //auto table = (std::string)
    // "{\"board\":[[1,1,1,1,1],[4,4,4,4,4],[4,4,3,4,4],[4,4,4,4,4],[2,2,2,2,2]]}";
    // auto table = (std::string)
    // "{\"board\":[[2,4,4,1,2],[3,4,2,4,4],[2,1,4,4,4],[4,4,4,2,4],[4,4,1,1,1]]}";
    // auto *board = new Board(getTable(table));

    // std::string fullMovesStr =
    //                         //     "c3 b4 b1 b3\n"
    //                         // //    "b4 d2 c5 c2\n"
    //                         //    "d2 e2 d1 d4\n"
    //                         // //    "e2 e4 b5 e2\n"
    //                         //    "e4 e3 d4 c5\n"
    //                         // //    "e3 e4 c2 d3\n"
    //                         //    "e4 e3 b3 b5\n"
    //                         // //    "e3 e4 d3 b1\n"
    //                         //    "e4 a4 a1 d4\n"
    //                         // //    "a4 c4 e2 d1\n"
    //                         //    "c4 a2 c1 c4\n"
    //                         // //    "a2 b3 a5 a1\n"
    //                         //    "b3 e3 e1 a5\n"
    //                         // //    "e3 a3 b1 c1\n"
    //                         //    "a3 e3 c5 a3\n"
    //                         // //    "e3 d2 e5 e1\n"
    //                         //    "d2 a2 d4 d2\n"
    //                         // //    "a2 b3 a1 b1\n"
    //                         //    "b3 a2 b5 b2\n"
    //                         // //    "a2 b3 b1 a2\n"
    //                         //    "b3 c2 b2 e5\n"
    //                         // //    "c2 a4 a2 b3\n"
    //                            "a4 b4 c4 b5\n";
    //                         //    "b4 a4 d5 d3";

    // std::regex ws("\\s+");
    // std::regex eol("\\n");
    // for (
    //     auto it = std::sregex_token_iterator(fullMovesStr.begin(),
    //     fullMovesStr.end(), eol, -1); it != std::sregex_token_iterator();
    //     ++it)
    // {
    //     std::string line = *it;

    //     printf("---------------------------------\n");
    //     std::cout << *board << std::endl;
    //     printf("---------------------------------\n");

    //     auto *fm = processInput(line);
    //     board->applyFullMove(fm);

    //     printf("---------------------------------\n");
    //     std::cout << line << std::endl;
    //     std::cout << *fm << std::endl;
    //     std::cout << *board << std::endl;
    //     printf("---------------------------------\n");

    //     auto t1 = std::chrono::system_clock::now();
    //     auto *fm_max = maxValue(board, 5, std::numeric_limits<int>::min(),
    //     std::numeric_limits<int>::max(), PieceKind::BLACK); auto t2 =
    //     std::chrono::system_clock::now(); board->applyFullMove(fm_max);

    //     printf("---------------------------------\n");
    //     std::cout << "time: " <<
    //     std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count() <<
    //     "s" << std::endl;
    //     // std::cout << *fm << std::endl;
    //     std::cout << *fm_max << std::endl;
    //     std::cout << *board << std::endl;
    //     printf("---------------------------------++++++++++++++++++++++++++\n");

    //     delete fm;
    //     delete fm_max;
    // }

    // delete board;

    return EXIT_SUCCESS;
}

// FullMove *processInput(std::string &input) {
//     // std::regex ws("\\s+");
//     //
//     // auto it = std::sregex_token_iterator(input.begin(), input.end(), ws, -1);
//     // std::string token = *it;
//     // auto *m1 = new Move(row2Int(token[1]), column2Int(token[0]), PieceKind::NEUTRON);
//     // ++it;
//     //
//     // token = *it;
//     // auto *m2 = new Move(row2Int(token[1]), column2Int(token[0]), PieceKind::NEUTRON);
//     // ++it;
//     //
//     // token = *it;
//     // auto *m3 = new Move(row2Int(token[1]), column2Int(token[0]), PieceKind::WHITE);
//     // ++it;
//     //
//     // token = *it;
//     // auto *m4 = new Move(row2Int(token[1]), column2Int(token[0]), PieceKind::WHITE);
//     // ++it;
//     //
//     // return new FullMove({m1, m2, m3, m4}, 0);
// }

// int column2Int(char c) {
//     if (c == 'a')
//         return 0;
//     if (c == 'b')
//         return 1;
//     if (c == 'c')
//         return 2;
//     if (c == 'd')
//         return 3;
//     if (c == 'e')
//         return 4;
//
//     return 0;
// }
//
// int row2Int(char r) {
//     if (r == '5')
//         return 0;
//     if (r == '4')
//         return 1;
//     if (r == '3')
//         return 2;
//     if (r == '2')
//         return 3;
//     if (r == '1')
//         return 4;
//
//     return 0;
// }
