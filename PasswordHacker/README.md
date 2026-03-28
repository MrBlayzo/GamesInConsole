# Password Hacker
A one-player console game where you need to find a complex password with 16 characters and a dictionary with 96 characters.

# Rules

### Manual Hacking

- Type the masked password character by character.
- Correct guess → earn money based on password complexity.
- Wrong guess → reset and try again.

### Automated Solvers

Buy assistants that crack passwords in background:

| Solver            | Strategy                            |
| ----------------- | ----------------------------------- |
| **Random Solver** | Generates random passwords          |
| **Brute Solver**  | Systematic enumeration (no repeats) |

### Menu Options

- **Password Complexity**: Adjust target password length (1–64) and character set complexity (0–3).
- **Shop**: Buy Random or Brute solvers with configurable `len`, `complexity`, and `hashrate`.

## Controls

| Key                                  | Action                                 |
| ------------------------------------ | -------------------------------------- |
| `A-Z`, `a-z`, `0-9`, `special chars` | Type password characters               |
| `Backspace`                          | Delete last character                  |
| `Esc`                                | Open main menu                         |
| `w` / `s` / `Enter`                  | Navigate menus                         |
| `r`/`f`                              | Increase/Decrease                      |
| `t`/`g`                              | Increase/Decrease by multiplying by 10 |

### Password complexity

| Complexity | Dictionary                                                                                        |
| ---------- | ------------------------------------------------------------------------------------------------- |
| 0          | 1234567890                                                                                        |
| 1          | 1234567890abcdefghijklmnopqrstuvwxyz                                                              |
| 2          | 1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ                                    |
| 3          | 1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]{};:'\"<,.>/?-_=+`~!@#$%^&*()\\\| |


# Building

Requirements:
1) A C++20-compatible compiler (MSVC recommended)
2) CMake 3.14 or higher

# Usage
Run the game