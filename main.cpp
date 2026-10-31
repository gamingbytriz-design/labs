#include <iostream>
#include <limits>
#include <cstdlib>
#include <string>
#include "core.hpp"

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void playManual(GameSession& session, IRouletteGenerator& generator, IJournalRepository& journal) {
    std::cout << "\n=== MANUAL MODE ===\n";
    while (true) {
        Money balance = session.getPlayer().getBalance();
        if (balance.getCents() <= 0) {
            std::cout << "You are bankrupt! Game over.\n";
            break;
        }

        std::cout << "\n--- NEW ROUND ---\n";
        std::cout << "Your balance: " << balance.toString() << " RUB\n";
        std::cout << "1. Bet on RED\n";
        std::cout << "2. Bet on BLACK\n";
        std::cout << "3. Bet on NUMBER\n";
        std::cout << "0. Back to menu\n";
        std::cout << "Choose option: ";

        int choice;
        if (!(std::cin >> choice)) {
            clearInput();
            continue;
        }

        if (choice == 0) break;

        long long amountRub;
        std::cout << "Enter bet amount (in RUB): ";
        if (!(std::cin >> amountRub)) {
            clearInput();
            std::cout << "Invalid input.\n";
            continue;
        }

        Money betAmount(amountRub * 100);
        Bet bet(BetType::COLOR, 0, Color::RED, betAmount);

        if (choice == 1) {
            bet = Bet(BetType::COLOR, 0, Color::RED, betAmount);
        }
        else if (choice == 2) {
            bet = Bet(BetType::COLOR, 0, Color::BLACK, betAmount);
        }
        else if (choice == 3) {
            int num;
            std::cout << "Enter number (0-36): ";
            if (!(std::cin >> num)) {
                clearInput();
                std::cout << "Invalid number.\n";
                continue;
            }
            bet = Bet(BetType::NUMBER, num, Color::GREEN, betAmount);
        }
        else {
            std::cout << "Invalid option.\n";
            continue;
        }

        try {
            session.playRound(bet, generator, journal);
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
}

void playAuto(GameSession& session, IRouletteGenerator& generator, IJournalRepository& journal) {
    std::cout << "\n=== AUTO MODE (BOT) ===\n";
    std::cout << "Bot will bet 100 RUB on RED each round.\n";

    int rounds;
    std::cout << "Enter number of rounds: ";
    if (!(std::cin >> rounds)) {
        clearInput();
        return;
    }

    for (int i = 0; i < rounds; ++i) {
        Money balance = session.getPlayer().getBalance();
        if (balance.getCents() <= 0) {
            std::cout << "Bot is bankrupt! Game over.\n";
            break;
        }

        Money betAmount(10000);
        if (balance < betAmount) {
            betAmount = balance;
        }

        Bet bet(BetType::COLOR, 0, Color::RED, betAmount);

        try {
            session.playRound(bet, generator, journal);
            std::cout << "Round " << i + 1 << " done. Balance: "
                << session.getPlayer().getBalance().toString() << " RUB\n";
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    unsigned int seed = 42;
    if (argc > 1) {
        seed = std::stoul(argv[1]);
    }

    try {
        Player player("Artyom", Money(100000));
        Casino casino(Money(10000000));
        GameSession session(player, casino, Money(1000), Money(50000));

        RouletteGenerator generator(seed);
        CSVJournalRepository journal("game_log.csv");

        if (argc > 2 && std::string(argv[2]) == "auto") {
            for (int i = 0; i < 10; ++i) {
                Money balance = session.getPlayer().getBalance();
                if (balance.getCents() <= 0) break;

                Money betAmount(10000);
                if (balance < betAmount) betAmount = balance;

                Bet bet(BetType::COLOR, 0, Color::RED, betAmount);

                try {
                    session.playRound(bet, generator, journal);
                }
                catch (...) {
                    break;
                }
            }
            std::cout << "FINAL_BALANCE: " << session.getPlayer().getBalance().getCents() << std::endl;
            return 0;
        }

        std::cout << "=== CASINO SIMULATOR ===\n";
        std::cout << "Starting player balance: " << player.getBalance().toString() << " RUB\n";

        while (true) {
            std::cout << "\n=== MAIN MENU ===\n";
            std::cout << "1. Manual mode\n";
            std::cout << "2. Auto mode (Bot)\n";
            std::cout << "0. Exit\n";
            std::cout << "Choose option: ";

            int choice;
            if (!(std::cin >> choice)) {
                clearInput();
                continue;
            }

            if (choice == 1) {
                playManual(session, generator, journal);
            }
            else if (choice == 2) {
                playAuto(session, generator, journal);
            }
            else if (choice == 0) {
                break;
            }
            else {
                std::cout << "Invalid option.\n";
            }
        }

        std::cout << "\nGame over. Data saved to game_log.csv\n";
        std::cout << "FINAL_BALANCE: " << session.getPlayer().getBalance().getCents() << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
    }
    return 0;
}