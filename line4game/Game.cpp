#include <string>
#include <iostream>
#include "Game.h"

char to_char(Participant p)
{
    switch (p)
    {
    case Participant::player1:
        return '*';
    case Participant::player2:
        return 'O';
    case Participant::none:
        return ' ';
    }
    return '?';
}

Board::Board(int width, int height)
    : width(width),
      height(height),
      engine(),
      board(height, std::vector<Participant>(width, Participant::none)) {}

int Board::get_new_cursor_pos(int cursor)
{
    draw(cursor);
    std::string input = engine.get();
    while (!input.empty())
    {
        for (const auto &c : input)
        {
            if (c == 'a')
            {
                cursor = std::max(cursor - 1, 0);
            }
            else if (c == 'd')
            {
                cursor = std::min(cursor + 1, width - 1);
            }
        }
        draw(cursor);
        input = engine.get();
    }
    return cursor;
}

void Board::draw(int cursor)
{
    engine.clear();
    for (int i = 0; i <= cursor * 2; ++i)
    {
        engine.print(' ');
    }
    engine.print('v');
    for (int i = cursor * 2; i <= (width + 1) * 2; ++i)
    {
        engine.print(' ');
    }
    engine.print('\n');
    for (const auto &row : board)
    {
        for (const auto &p : row)
        {
            engine.print('|', to_char(p));
        }
        engine.print('|', '\n');
    }
}

bool Board::try_add_piece(int cursor, Participant p)
{
    if (board[0][cursor] != Participant::none)
        return false;
    int row = 0;
    while (row < height && board[row][cursor] == Participant::none)
        ++row;
    board[row - 1][cursor] = p;
    draw(cursor);
    return true;
}

void Board::set_winner(Participant p)
{
    // engine.clear();
    if (p == Participant::player1)
        engine.print("Player 1 win!!!");
    else
        engine.print("Player 2 win!!!");
}

Line4Game::Line4Game(int width, int height)
    : board(width, height),
      player1(std::make_unique<PeoplePlayer>(Participant::player1)),
      player2(std::make_unique<PeoplePlayer>(Participant::player2)) {}
Line4Game::Line4Game(std::unique_ptr<Player> player2,
                     int width, int height)
    : board(width, height),
      player1(std::make_unique<PeoplePlayer>(Participant::player1)),
      player2(std::move(player2)) {}
Line4Game::Line4Game(std::unique_ptr<Player> player2,
                     std::unique_ptr<Player> player1,
                     int width, int height)
    : board(width, height),
      player1(std::move(player1)),
      player2(std::move(player2)) {}

void Line4Game::play()
{
    Participant winner;
    while (true)
    {
        player1->move(board);
        winner = check_win();
        if (winner != Participant::none)
        {
            board.set_winner(winner);
            return;
        }

        player2->move(board);
        winner = check_win();
        if (winner != Participant::none)
        {
            board.set_winner(winner);
            return;
        }
    }
}

Participant Line4Game::check_win()
{
    if (auto result = board.check_row_win(); result != Participant::none)
        return result;
    if (auto result = board.check_col_win(); result != Participant::none)
        return result;
    if (auto result = board.check_diag1_win(); result != Participant::none)
        return result;
    if (auto result = board.check_diag2_win(); result != Participant::none)
        return result;
    return Participant::none;
}
Participant Board::check_win_for_current_pos(Participant &prev, Participant &curr, int &player1_count, int &player2_count)
{
    if (curr != prev)
    {
        switch (curr)
        {
        case Participant::player1:
            player1_count = 1;
            break;
        case Participant::player2:
            player2_count = 1;
            break;
        default:
            player1_count = 0;
            player2_count = 0;
            break;
        }
    }
    else
    {
        switch (curr)
        {
        case Participant::player1:
            ++player1_count;
            break;
        case Participant::player2:
            ++player2_count;
            break;
        default:
            break;
        }
    }
    if (player1_count >= 4)
        return Participant::player1;
    if (player2_count >= 4)
        return Participant::player2;
    return Participant::none;
}
Participant Board::check_row_win()
{
    for (int row = 0; row < height; ++row)
    {
        int player1_count = 0;
        int player2_count = 0;
        Participant prev = Participant::none;
        for (int col = 0; col < width; ++col)
        {
            Participant &curr = board[row][col];
            Participant winner = check_win_for_current_pos(prev, curr, player1_count, player2_count);
            if (winner != Participant::none)
                return winner;
            prev = curr;
        }
    }
    return Participant::none;
}
Participant Board::check_col_win()
{
    for (int col = 0; col < width; ++col)
    {
        int player1_count = 0;
        int player2_count = 0;
        Participant prev = Participant::none;
        for (int row = 0; row < height; ++row)
        {
            Participant &curr = board[row][col];
            Participant winner = check_win_for_current_pos(prev, curr, player1_count, player2_count);
            if (winner != Participant::none)
                return winner;
            prev = curr;
        }
    }
    return Participant::none;
}
Participant Board::check_diag1_win()
{
    for (int d = 3; d < width + height - 1 - 3; ++d)
    {
        int row = std::max(0, height - d - 1);
        int col = std::max(0, d - height + 1);
        int player1_count = 0;
        int player2_count = 0;
        Participant prev = Participant::none;
        for (int i = 0; i < std::min(height-row, width-col); ++i)
        {
            Participant &curr = board[row+i][col+i];
            Participant winner = check_win_for_current_pos(prev, curr, player1_count, player2_count);
            if (winner != Participant::none)
                return winner;
            prev = curr;
        }
    }
    return Participant::none;
}
Participant Board::check_diag2_win()
{
    for (int d = 3; d < width + height - 1 - 3; ++d)
    {
        int row = std::min(height - 1, d);
        int col = std::max(0, d - height + 1);
        int player1_count = 0;
        int player2_count = 0;
        Participant prev = Participant::none;
        for (int i = 0; i < std::min(row+1, width-col); ++i)
        {
            Participant &curr = board[row-i][col+i];
            Participant winner = check_win_for_current_pos(prev, curr, player1_count, player2_count);
            if (winner != Participant::none)
                return winner;
            prev = curr;
        }
    }
    return Participant::none;
}

Player::Player(Participant participant) : participant(participant) {}

PeoplePlayer::PeoplePlayer(Participant p) : Player(p) {}

ComputerPlayer::ComputerPlayer(Participant p, ComputeParams params)
    : Player(p), compute_params(params) {}

void PeoplePlayer::move(Board &board)
{
    bool valid_move = false;
    while (!valid_move)
    {
        cursor = board.get_new_cursor_pos(cursor);
        valid_move = board.try_add_piece(cursor, participant);
    }
}

void ComputerPlayer::move(Board &board)
{
    bool valid_move = false;
}