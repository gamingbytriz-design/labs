#include <gtest/gtest.h>
#include <memory>
#include "core.hpp"

TEST(MoneyTest, BasicMath) {
    Money a(100);
    Money b(250);
    EXPECT_EQ((a + b).getCents(), 350);
    EXPECT_EQ((b - a).getCents(), 150);
    EXPECT_EQ((a * 3).getCents(), 300);
}

TEST(MoneyTest, Comparisons) {
    Money a(100);
    Money b(250);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a == Money(100));
    EXPECT_TRUE(b >= Money(250));
}

TEST(MoneyTest, Formatting) {
    Money a(1050);
    EXPECT_EQ(a.toString(), "10.50");
    EXPECT_EQ(a.getRubles(), 10.5);
}

TEST(PlayerTest, DepositWithdraw) {
    Player p("Artyom", Money(1000));
    p.deposit(Money(500));
    EXPECT_EQ(p.getBalance().getCents(), 1500);
    p.withdraw(Money(200));
    EXPECT_EQ(p.getBalance().getCents(), 1300);
    EXPECT_THROW(p.withdraw(Money(2000)), std::invalid_argument);
}

TEST(CasinoTest, PayoutLimits) {
    Casino c(Money(10000));
    c.deposit(Money(5000));
    EXPECT_EQ(c.getBalance().getCents(), 15000);
    c.withdraw(Money(1000));
    EXPECT_EQ(c.getBalance().getCents(), 14000);
    EXPECT_THROW(c.withdraw(Money(20000)), std::invalid_argument);
}

TEST(GameSessionTest, ColorLogic) {
    EXPECT_EQ(GameSession::getColorForNumber(0), Color::GREEN);
    EXPECT_EQ(GameSession::getColorForNumber(1), Color::RED);
    EXPECT_EQ(GameSession::getColorForNumber(2), Color::BLACK);
    EXPECT_EQ(GameSession::getColorForNumber(11), Color::BLACK);
    EXPECT_EQ(GameSession::getColorForNumber(12), Color::RED);
    EXPECT_EQ(GameSession::getColorForNumber(19), Color::RED);
}

class FakeGenerator : public IRouletteGenerator {
public:
    int fixedNumber;
    FakeGenerator(int n) : fixedNumber(n) {}
    int spin() override { return fixedNumber; }
};

class FakeJournal : public IJournalRepository {
public:
    void log(int round, const IBet& bet, int result, Money win, Money playerBal, Money casinoBal) override {}
};

TEST(GameSessionTest, WinAndLoss) {
    Player p("Artyom", Money(10000));
    Casino c(Money(100000));
    GameSession session(p, c, Money(100), Money(5000));

    FakeJournal journal;

    FakeGenerator winGen(1);
    auto bet = std::make_shared<ColorBet>(Color::RED, Money(1000));
    session.playRound(bet, winGen, journal);
    EXPECT_EQ(session.getPlayer().getBalance().getCents(), 11000);

    FakeGenerator loseGen(1);
    auto loseBet = std::make_shared<ColorBet>(Color::BLACK, Money(1000));
    session.playRound(loseBet, loseGen, journal);
    EXPECT_EQ(session.getPlayer().getBalance().getCents(), 10000);

    FakeGenerator numGen(36);
    auto numBet = std::make_shared<NumberBet>(36, Money(100));
    session.playRound(numBet, numGen, journal);
    EXPECT_EQ(session.getPlayer().getBalance().getCents(), 13500);
}

TEST(GameSessionTest, ThrowsOnInvalidBet) {
    Player p("Artyom", Money(100));
    Casino c(Money(10000));
    GameSession session(p, c, Money(200), Money(5000));

    FakeJournal journal;
    FakeGenerator gen(1);

    auto smallBet = std::make_shared<ColorBet>(Color::RED, Money(100));
    EXPECT_THROW(session.playRound(smallBet, gen, journal), std::invalid_argument);

    auto highBet = std::make_shared<ColorBet>(Color::RED, Money(300));
    EXPECT_THROW(session.playRound(highBet, gen, journal), std::invalid_argument);
}

/*int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}*/