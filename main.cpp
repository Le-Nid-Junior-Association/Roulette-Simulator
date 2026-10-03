// =====================================================================
//  ROULETTE SIMULATOR
//  Simulateur de roulette europeenne en console (pas d'argent reel,
//  pas de mise, pas de recompense : uniquement des statistiques).
//  Projet realise par Lucas & Florian pour apprendre le C++.
// =====================================================================

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <fstream>
#include <iomanip>
#include <limits>
#include <algorithm>

const int MAX_NUMBER = 36;            // roulette europeenne : 0 a 36
const int TOTAL_NUMBERS = 37;
const int HISTORY_DISPLAY_LIMIT = 20; // on n'affiche que les N derniers tirages
const long long MAX_SIMULATION_SPINS = 1000000;
const std::string EXPORT_FILE_NAME = "roulette_statistics.txt";

// Numeros rouges d'une roulette europeenne. Les autres (sauf 0) sont noirs.
const std::vector<int> RED_NUMBERS = {
    1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36
};

struct Spin {
    int number;
    std::string color;
};

// Toutes les statistiques de la session sont regroupees ici,
// pour eviter les variables globales eparpillees.
struct RouletteStats {
    std::vector<Spin> history;
    std::array<int, TOTAL_NUMBERS> numberCounts{};
    long long totalSpins = 0;
    long long redCount = 0;
    long long blackCount = 0;
    long long greenCount = 0;
    long long sumOfNumbers = 0;

    int currentRedStreak = 0, longestRedStreak = 0;
    int currentBlackStreak = 0, longestBlackStreak = 0;
    int currentNonZeroStreak = 0, longestNonZeroStreak = 0;
};

// --- Prototypes ---
void ShowMenu();
int ReadInt(const std::string& prompt, long long minValue, long long maxValue);

int SpinRoulette(std::mt19937& rng);
std::string GetColor(int number);
void RecordSpin(RouletteStats& stats, int number);

void ShowSpinResult(int number, const std::string& color);
void ShowStatistics(const RouletteStats& stats);
void ShowNumberStatistics(const RouletteStats& stats);
void ShowColorStatistics(const RouletteStats& stats);
void ShowHistory(const RouletteStats& stats);
void RunSimulation(RouletteStats& stats, std::mt19937& rng);
void ResetStatistics(RouletteStats& stats);
void ExportStatistics(const RouletteStats& stats);

double Percentage(long long part, long long total);
double TheoreticalFrequency(const std::string& color);
void PrintColorBlock(const std::string& name, long long count, long long total, bool showTheoretical);
void FindMostAndLeastFrequent(const RouletteStats& stats, int& mostNumber, int& mostCount, int& leastNumber, int& leastCount);

// =====================================================================
//  MAIN
// =====================================================================

int main() {
    // Source d'entropie reelle : jamais de valeur fixe (seed) codee en dur.
    std::random_device randomDevice;
    std::mt19937 rng(randomDevice());

    RouletteStats stats;
    bool running = true;

    while (running) {
        ShowMenu();
        int choice = ReadInt("", 1, 9);

        if (choice == 1) {
            int number = SpinRoulette(rng);
            RecordSpin(stats, number);
            ShowSpinResult(number, GetColor(number));
        } else if (choice == 2) {
            ShowStatistics(stats);
        } else if (choice == 3) {
            RunSimulation(stats, rng);
        } else if (choice == 4) {
            ShowNumberStatistics(stats);
        } else if (choice == 5) {
            ShowColorStatistics(stats);
        } else if (choice == 6) {
            ShowHistory(stats);
        } else if (choice == 7) {
            ResetStatistics(stats);
        } else if (choice == 8) {
            ExportStatistics(stats);
        } else {
            std::cout << "\nThanks for using Roulette Simulator. Goodbye!\n";
            running = false;
        }
    }

    return 0;
}

// =====================================================================
//  MENU / ENTREES UTILISATEUR
// =====================================================================

void ShowMenu() {
    std::cout << "\n========================================\n";
    std::cout << "          ROULETTE SIMULATOR\n";
    std::cout << "========================================\n\n";
    std::cout << "1. Spin roulette\n";
    std::cout << "2. View statistics\n";
    std::cout << "3. Run simulation\n";
    std::cout << "4. Number statistics\n";
    std::cout << "5. Color statistics\n";
    std::cout << "6. View history\n";
    std::cout << "7. Reset statistics\n";
    std::cout << "8. Export statistics\n";
    std::cout << "9. Exit\n\n";
    std::cout << "Choose an option: ";
}

// Lit un entier compris entre minValue et maxValue.
// Reprompte tant que l'entree n'est pas valide (texte, hors limites...).
int ReadInt(const std::string& prompt, long long minValue, long long maxValue) {
    if (!prompt.empty()) {
        std::cout << prompt;
    }

    long long value = 0;
    while (true) {
        std::cin >> value;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number between "
                       << minValue << " and " << maxValue << ": ";
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (value < minValue || value > maxValue) {
            std::cout << "Please enter a number between " << minValue
                       << " and " << maxValue << ": ";
            continue;
        }

        return static_cast<int>(value);
    }
}

// =====================================================================
//  ROULETTE : TIRAGE, COULEUR, ENREGISTREMENT
// =====================================================================

int SpinRoulette(std::mt19937& rng) {
    std::uniform_int_distribution<int> distribution(0, MAX_NUMBER);
    return distribution(rng);
}

std::string GetColor(int number) {
    if (number == 0) {
        return "GREEN";
    }
    bool isRed = std::find(RED_NUMBERS.begin(), RED_NUMBERS.end(), number) != RED_NUMBERS.end();
    return isRed ? "RED" : "BLACK";
}

// Met a jour l'historique, les compteurs, la moyenne et les series
// pour un nouveau tirage. Utilise aussi bien par un tirage simple
// que par la simulation.
void RecordSpin(RouletteStats& stats, int number) {
    std::string color = GetColor(number);

    stats.history.push_back({number, color});
    stats.numberCounts[number]++;
    stats.totalSpins++;
    stats.sumOfNumbers += number;

    if (color == "RED") {
        stats.redCount++;
        stats.currentRedStreak++;
        stats.currentBlackStreak = 0;
    } else if (color == "BLACK") {
        stats.blackCount++;
        stats.currentBlackStreak++;
        stats.currentRedStreak = 0;
    } else {
        stats.greenCount++;
        stats.currentRedStreak = 0;
        stats.currentBlackStreak = 0;
    }
    stats.currentNonZeroStreak = (color != "GREEN") ? stats.currentNonZeroStreak + 1 : 0;

    stats.longestRedStreak = std::max(stats.longestRedStreak, stats.currentRedStreak);
    stats.longestBlackStreak = std::max(stats.longestBlackStreak, stats.currentBlackStreak);
    stats.longestNonZeroStreak = std::max(stats.longestNonZeroStreak, stats.currentNonZeroStreak);
}

void ShowSpinResult(int number, const std::string& color) {
    std::cout << "\n========================================\n";
    std::cout << "RESULT\n\n";
    std::cout << "Number: " << number << "\n";
    std::cout << "Color: " << color << "\n";
    std::cout << "========================================\n";
}

// =====================================================================
//  OUTILS DE CALCUL (pourcentages, probabilites theoriques)
// =====================================================================

double Percentage(long long part, long long total) {
    if (total == 0) return 0.0;
    return (static_cast<double>(part) / static_cast<double>(total)) * 100.0;
}

double TheoreticalFrequency(const std::string& color) {
    if (color == "GREEN") return (1.0 / 37.0) * 100.0;
    return (18.0 / 37.0) * 100.0; // RED et BLACK ont la meme probabilite
}

// Affiche le compte/frequence d'une couleur, avec en option la
// comparaison a la probabilite theorique.
void PrintColorBlock(const std::string& name, long long count, long long total, bool showTheoretical) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << name << "\n";
    std::cout << "Count: " << count << "\n";
    std::cout << "Frequency: " << Percentage(count, total) << "%\n";
    if (showTheoretical) {
        std::cout << "Theoretical: " << TheoreticalFrequency(name) << "%\n";
    }
    std::cout << "\n";
}

// Parcourt les compteurs par numero pour trouver le plus et le moins frequent.
void FindMostAndLeastFrequent(const RouletteStats& stats, int& mostNumber, int& mostCount, int& leastNumber, int& leastCount) {
    mostNumber = 0;
    leastNumber = 0;
    mostCount = stats.numberCounts[0];
    leastCount = stats.numberCounts[0];

    for (int number = 1; number <= MAX_NUMBER; ++number) {
        int count = stats.numberCounts[number];
        if (count > mostCount) { mostCount = count; mostNumber = number; }
        if (count < leastCount) { leastCount = count; leastNumber = number; }
    }
}

// =====================================================================
//  AFFICHAGE DES STATISTIQUES
// =====================================================================

void ShowStatistics(const RouletteStats& stats) {
    std::cout << "\n========================================\n";
    std::cout << "            STATISTICS\n";
    std::cout << "========================================\n\n";

    if (stats.totalSpins == 0) {
        std::cout << "No spins recorded yet. Try option 1 or 3 first.\n";
        std::cout << "========================================\n";
        return;
    }

    std::cout << "Total spins: " << stats.totalSpins << "\n\n";
    PrintColorBlock("RED", stats.redCount, stats.totalSpins, false);
    PrintColorBlock("BLACK", stats.blackCount, stats.totalSpins, false);
    PrintColorBlock("GREEN", stats.greenCount, stats.totalSpins, false);

    double average = static_cast<double>(stats.sumOfNumbers) / static_cast<double>(stats.totalSpins);
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Average number: " << average << "\n";
    std::cout << "========================================\n\n";

    // "Same color" streak = plus longue serie de rouge OU de noir d'affilee.
    std::cout << "SEQUENCES\n\n";
    std::cout << "Longest RED streak: " << stats.longestRedStreak << "\n";
    std::cout << "Longest BLACK streak: " << stats.longestBlackStreak << "\n";
    std::cout << "Longest non-zero streak: " << stats.longestNonZeroStreak << "\n";
    std::cout << "Longest same-color streak: "
               << std::max(stats.longestRedStreak, stats.longestBlackStreak) << "\n\n";
    std::cout << "Note: these are only observations, they cannot predict\n";
    std::cout << "the next spin. The roulette has no memory.\n";
}

void ShowNumberStatistics(const RouletteStats& stats) {
    std::cout << "\n========================================\n";
    std::cout << "        NUMBER STATISTICS\n";
    std::cout << "========================================\n\n";

    if (stats.totalSpins == 0) {
        std::cout << "No spins recorded yet. Try option 1 or 3 first.\n";
        std::cout << "========================================\n";
        return;
    }

    for (int number = 0; number <= MAX_NUMBER; ++number) {
        std::cout << std::setw(2) << number << " : " << stats.numberCounts[number] << "\n";
    }

    int mostNumber, mostCount, leastNumber, leastCount;
    FindMostAndLeastFrequent(stats, mostNumber, mostCount, leastNumber, leastCount);

    std::cout << "\nMost frequent number: " << mostNumber << " (Occurrences: " << mostCount << ")\n";
    std::cout << "Least frequent number: " << leastNumber << " (Occurrences: " << leastCount << ")\n\n";
    std::cout << "Note: this does not make a number more or less likely\n";
    std::cout << "to appear next. Every spin is independent.\n";
    std::cout << "========================================\n";
}

void ShowColorStatistics(const RouletteStats& stats) {
    std::cout << "\n========================================\n";
    std::cout << "        COLOR STATISTICS\n";
    std::cout << "========================================\n\n";

    if (stats.totalSpins == 0) {
        std::cout << "No spins recorded yet. Try option 1 or 3 first.\n";
        std::cout << "========================================\n";
        return;
    }

    std::cout << "Total spins: " << stats.totalSpins << "\n\n";
    std::cout << "--- Observed vs Theoretical ---\n\n";
    PrintColorBlock("RED", stats.redCount, stats.totalSpins, true);
    PrintColorBlock("BLACK", stats.blackCount, stats.totalSpins, true);
    PrintColorBlock("GREEN", stats.greenCount, stats.totalSpins, true);

    std::cout << "Note: observed frequencies fluctuate around the theoretical\n";
    std::cout << "values because of randomness.\n";
    std::cout << "========================================\n";
}

void ShowHistory(const RouletteStats& stats) {
    std::cout << "\n========================================\n";
    std::cout << "              HISTORY\n";
    std::cout << "========================================\n\n";

    if (stats.history.empty()) {
        std::cout << "No spins recorded yet.\n";
        std::cout << "========================================\n";
        return;
    }

    long long total = static_cast<long long>(stats.history.size());
    long long start = (total > HISTORY_DISPLAY_LIMIT) ? total - HISTORY_DISPLAY_LIMIT : 0;

    if (start > 0) {
        std::cout << "(Showing the last " << HISTORY_DISPLAY_LIMIT << " spins out of " << total << ")\n\n";
    }

    for (long long i = start; i < total; ++i) {
        const Spin& spin = stats.history[static_cast<size_t>(i)];
        std::cout << (i + 1) << ". " << spin.number << " - " << spin.color << "\n";
    }
    std::cout << "========================================\n";
}

// =====================================================================
//  SIMULATION AUTOMATIQUE
// =====================================================================

void RunSimulation(RouletteStats& stats, std::mt19937& rng) {
    std::cout << "\nHow many spins? (e.g. 100, 1000, 10000, 100000)\n";
    int spinsRequested = ReadInt("> ", 1, MAX_SIMULATION_SPINS);

    std::cout << "\nRunning simulation...\n\n";

    // On affiche la progression environ 10 fois, meme pour un gros total.
    long long step = std::max<long long>(1, spinsRequested / 10);

    for (int i = 1; i <= spinsRequested; ++i) {
        RecordSpin(stats, SpinRoulette(rng));
        if (i % step == 0 || i == spinsRequested) {
            std::cout << i << " / " << spinsRequested << "\n";
        }
    }

    std::cout << "\nSimulation complete.\n";

    // On reutilise les ecrans deja existants : pas de code duplique.
    ShowStatistics(stats);
    std::cout << "\n";

    int mostNumber, mostCount, leastNumber, leastCount;
    FindMostAndLeastFrequent(stats, mostNumber, mostCount, leastNumber, leastCount);
    std::cout << "Most frequent number overall: " << mostNumber << " (Occurrences: " << mostCount << ")\n";
    std::cout << "Least frequent number overall: " << leastNumber << " (Occurrences: " << leastCount << ")\n";
    std::cout << "========================================\n";
}

// =====================================================================
//  RESET
// =====================================================================

void ResetStatistics(RouletteStats& stats) {
    std::cout << "\nAre you sure you want to reset all statistics?\n\n";
    std::cout << "1. Yes\n2. No\n\n";

    int confirmation = ReadInt("Choose an option: ", 1, 2);

    if (confirmation == 1) {
        stats = RouletteStats();
        std::cout << "\nAll statistics have been reset.\n";
    } else {
        std::cout << "\nReset cancelled.\n";
    }
}

// =====================================================================
//  EXPORT DES STATISTIQUES
// =====================================================================

void ExportStatistics(const RouletteStats& stats) {
    if (stats.totalSpins == 0) {
        std::cout << "\nNothing to export yet. Spin the roulette or run a simulation first.\n";
        return;
    }

    std::ofstream file(EXPORT_FILE_NAME);
    if (!file.is_open()) {
        std::cout << "\nError: could not create the export file.\n";
        return;
    }

    file << std::fixed << std::setprecision(2);
    file << "ROULETTE SIMULATOR - EXPORT\n";
    file << "========================================\n\n";
    file << "Total spins: " << stats.totalSpins << "\n\n";

    file << "RED   - Count: " << stats.redCount << " - Frequency: " << Percentage(stats.redCount, stats.totalSpins)
         << "% - Theoretical: " << TheoreticalFrequency("RED") << "%\n";
    file << "BLACK - Count: " << stats.blackCount << " - Frequency: " << Percentage(stats.blackCount, stats.totalSpins)
         << "% - Theoretical: " << TheoreticalFrequency("BLACK") << "%\n";
    file << "GREEN - Count: " << stats.greenCount << " - Frequency: " << Percentage(stats.greenCount, stats.totalSpins)
         << "% - Theoretical: " << TheoreticalFrequency("GREEN") << "%\n\n";

    double average = static_cast<double>(stats.sumOfNumbers) / static_cast<double>(stats.totalSpins);
    file << "Average number: " << average << "\n\n";

    file << "--- Number counts ---\n";
    for (int number = 0; number <= MAX_NUMBER; ++number) {
        file << std::setw(2) << number << " : " << stats.numberCounts[number] << "\n";
    }

    file << "\n--- Sequences ---\n";
    file << "Longest RED streak: " << stats.longestRedStreak << "\n";
    file << "Longest BLACK streak: " << stats.longestBlackStreak << "\n";
    file << "Longest non-zero streak: " << stats.longestNonZeroStreak << "\n";

    file.close();
    std::cout << "\nStatistics exported to \"" << EXPORT_FILE_NAME << "\".\n";
}
