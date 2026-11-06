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


ColorBet::ColorBet(Color c, Money a) : color(c), amount(a) {}
Money ColorBet::getAmount() const { return amount; }
Money ColorBet::getMaxPotentialWin() const { return amount * 2; }
bool ColorBet::isWin(int result, Color resultColor) const { return resultColor == color; }
std::string ColorBet::getBetDescription() const {
    return (color == Color::RED) ? "Color RED" : (color == Color::BLACK) ? "Color BLACK" : "Color GREEN";
}


NumberBet::NumberBet(int n, Money a) : number(n), amount(a) {}
Money NumberBet::getAmount() const { return amount; }
Money NumberBet::getMaxPotentialWin() const { return amount * 36; }
bool NumberBet::isWin(int result, Color resultColor) const { return result == number; }
std::string NumberBet::getBetDescription() const {
    return "Number " + std::to_string(number);
}

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

void CSVJournalRepository::log(int round, const IBet& bet, int result, Money win, Money playerBal, Money casinoBal) {
    file << round << "," << bet.getBetDescription() << "," << bet.getAmount().getRubles() << ","
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

void GameSession::playRound(std::shared_ptr<IBet> bet, IRouletteGenerator& generator, IJournalRepository& journal) {
    if (bet->getAmount() < minBet || bet->getAmount() > maxBet) {
        throw std::invalid_argument("Bet exceeds table limits");
    }
    if (player.getBalance() < bet->getAmount()) {
        throw std::invalid_argument("Insufficient funds");
    }
    if (casino.getBalance() < bet->getMaxPotentialWin()) {
        throw std::invalid_argument("Casino cannot cover this bet");
    }

    player.withdraw(bet->getAmount());
    int result = generator.spin();
    Color resultColor = getColorForNumber(result);

    Money win(0);

    if (bet->isWin(result, resultColor)) {
        win = bet->getMaxPotentialWin();
        casino.withdraw(win);
        player.deposit(win);
    }
    else {
        casino.deposit(bet->getAmount());
    }

    journal.log(currentRound, *bet, result, win, player.getBalance(), casino.getBalance());
    currentRound++;
}
