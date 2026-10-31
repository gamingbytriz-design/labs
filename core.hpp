#pragma once
#include <string>
#include <fstream>
#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>

class Money {
private:
    long long cents;
public:
    Money(long long c = 0);
    long long getCents() const;
    double getRubles() const;
    std::string toString() const;

    Money operator+(const Money& other) const;
    Money operator-(const Money& other) const;
    Money operator*(int multiplier) const;

    bool operator>=(const Money& other) const;
    bool operator>(const Money& other) const;
    bool operator<(const Money& other) const;
    bool operator==(const Money& other) const;
};

enum class BetType { COLOR, NUMBER };
enum class Color { RED, BLACK, GREEN };

struct Bet {
    BetType type;
    int number;
    Color color;
    Money amount;
    Bet();
    Bet(BetType t, int n, Color c, Money a);
};

class Player {
private:
    std::string name;
    Money balance;
public:
    Player(std::string n, Money startBalance);
    std::string getName() const;
    Money getBalance() const;
    void withdraw(Money m);
    void deposit(Money m);
};

class Casino {
private:
    Money balance;
public:
    Casino(Money startBalance);
    Money getBalance() const;
    void withdraw(Money m);
    void deposit(Money m);
};

class IRouletteGenerator {
public:
    virtual ~IRouletteGenerator() = default;
    virtual int spin() = 0;
};

class RouletteGenerator : public IRouletteGenerator {
private:
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;
public:
    RouletteGenerator(unsigned int seed);
    int spin() override;
};

class IJournalRepository {
public:
    virtual ~IJournalRepository() = default;
    virtual void log(int round, const Bet& bet, int result, Money win, Money playerBal, Money casinoBal) = 0;
};

class CSVJournalRepository : public IJournalRepository {
private:
    std::ofstream file;
public:
    CSVJournalRepository(const std::string& filename);
    ~CSVJournalRepository() override;
    void log(int round, const Bet& bet, int result, Money win, Money playerBal, Money casinoBal) override;
};

class GameSession {
private:
    Player player;
    Casino casino;
    Money minBet;
    Money maxBet;
    int currentRound;
public:
    GameSession(Player p, Casino c, Money minB, Money maxB);
    Player& getPlayer();
    Casino& getCasino();
    static Color getColorForNumber(int num);
    void playRound(const Bet& bet, IRouletteGenerator& generator, IJournalRepository& journal);
};
