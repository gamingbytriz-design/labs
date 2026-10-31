#include "core.hpp"

Money::Money(long long c) : cents(c) {}
long long Money::getCents() const { return cents; }
double Money::getRubles() const { return cents / 100.0; }

std::string Money::toString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << getRubles();
    return oss.str();
}

Money Money::operator+(const Money& other) const { return Money(cents + other.cents); }
Money Money::operator-(const Money& other) const { return Money(cents - other.cents); }
Money Money::operator*(int multiplier) const { return Money(cents * multiplier); }
bool Money::operator>=(const Money& other) const { return cents >= other.cents; }
bool Money::operator>(const Money& other) const { return cents > other.cents; }
bool Money::operator<(const Money& other) const { return cents < other.cents; }
bool Money::operator==(const Money& other) const { return cents == other.cents; }

Bet::Bet() : type(BetType::COLOR), number(0), color(Color::RED), amount(0) {}
Bet::Bet(BetType t, int n, Color c, Money a) : type(t), number(n), color(c), amount(a) {}

Player::Player(std::string n, Money startBalance) : name(n), balance(startBalance) {}
std::string Player::getName() const { return name; }
Money Player::getBalance() const { return balance; }
void Player::withdraw(Money m) {
    if (balance < m) throw std::invalid_argument("Insufficient player funds");
    balance = balance - m;
}
void Player::deposit(Money m) { balance = balance + m; }

Casino::Casino(Money startBalance) : balance(startBalance) {}
Money Casino::getBalance() const { return balance; }
void Casino::withdraw(Money m) {
    if (balance < m) throw std::invalid_argument("Casino cannot payout the win");
    balance = balance - m;
}
void Casino::deposit(Money m) { balance = balance + m; }

RouletteGenerator::RouletteGenerator(unsigned int seed) : rng(seed), dist(0, 36) {}
int RouletteGenerator::spin() { return dist(rng); }

CSVJournalRepository::CSVJournalRepository(const std::string& filename) {
    file.open(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open journal file");
    }
    file << "Round,BetType,BetAmount,Result,WinAmount,PlayerBalance,CasinoBalance\n";
}

CSVJournalRepository::~CSVJournalRepository() {
    if (file.is_open()) {
        file.close();
    }
}

void CSVJournalRepository::log(int round, const Bet& bet, int result, Money win, Money playerBal, Money casinoBal) {
    std::string typeStr = (bet.type == BetType::COLOR) ? "Color" : "Number";
    file << round << "," << typeStr << "," << bet.amount.getRubles() << ","
         << result << "," << win.getRubles() << ","
         << playerBal.getRubles() << "," << casinoBal.getRubles() << "\n";
}

GameSession::GameSession(Player p, Casino c, Money minB, Money maxB)
    : player(p), casino(c), minBet(minB), maxBet(maxB), currentRound(1) {}

Player& GameSession::getPlayer() { return player; }
Casino& GameSession::getCasino() { return casino; }

Color GameSession::getColorForNumber(int num) {
    if (num == 0) return Color::GREEN;
    if ((num >= 1 && num <= 10) || (num >= 19 && num <= 28)) {
        return (num % 2 == 0) ? Color::BLACK : Color::RED;
    }
    return (num % 2 == 0) ? Color::RED : Color::BLACK;
}

void GameSession::playRound(const Bet& bet, IRouletteGenerator& generator, IJournalRepository& journal) {
    if (bet.amount < minBet || bet.amount > maxBet) {
        throw std::invalid_argument("Bet exceeds table limits");
    }
    if (player.getBalance() < bet.amount) {
        throw std::invalid_argument("Insufficient funds");
    }

    Money maxPotentialWin = (bet.type == BetType::NUMBER) ? bet.amount * 36 : bet.amount * 2;
    if (casino.getBalance() < maxPotentialWin) {
        throw std::invalid_argument("Casino cannot cover this bet");
    }

    player.withdraw(bet.amount);
    int result = generator.spin();

    Money win(0);
    bool isWin = false;

    if (bet.type == BetType::COLOR && bet.color == getColorForNumber(result)) {
        win = bet.amount * 2;
        isWin = true;
    } else if (bet.type == BetType::NUMBER && bet.number == result) {
        win = bet.amount * 36;
        isWin = true;
    }

    if (isWin) {
        casino.withdraw(win);
        player.deposit(win);
    } else {
        casino.deposit(bet.amount);
    }

    journal.log(currentRound, bet, result, win, player.getBalance(), casino.getBalance());
    currentRound++;
}
