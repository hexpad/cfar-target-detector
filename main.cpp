#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <utility>

class ProfileGenerator
{
public:
    std::vector<double> generate(int size)
    {
        std::mt19937 generator(42);
        std::exponential_distribution<double> distribution(1.0);

        std::vector<double> profile(size);

        for (int i = 0; i < size; i++)
        {
            profile[i] = distribution(generator);
        }

        if (size > 45)
        {
            profile[20] += 20.0;
            profile[45] += 4.0;
        }

        return profile;
    }
};

class CfarDetector
{
public:
    CfarDetector(int trainingCells, int guardCells, double pfa)
        : m_trainingCells(trainingCells), m_guardCells(guardCells), m_pfa(pfa)
    {

    }

    void process(const std::vector<double>& profile)
    {
        std::cout << "\n--- CFAR Detection Results ---\n";
        std::cout << "Pfa = " << m_pfa << "\n\n";

        int size = static_cast<int>(profile.size());
        for (int i = 0; i < size; i++)
        {
            auto [noise, usedCells] = calculateNoiseAndCount(profile, i);
            double alpha = calculateThresholdMultiplier(usedCells);
            double threshold = noise * alpha;
            bool isTarget = profile[i] > threshold;

            printResult(i, profile[i], noise, threshold, isTarget);
        }
    }

    void exportToCsv(const std::vector<double>& profile, const std::string& fileName)
    {
        std::ofstream file(fileName);

        if (!file.is_open())
        {
            std::cout << "Failed to open file: " << fileName << "\n";
            return;
        }

        file << "index,value,noise,threshold,is_target\n";

        int size = static_cast<int>(profile.size());
        for (int i = 0; i < size; i++)
        {
            auto [noise, usedCells] = calculateNoiseAndCount(profile, i);
            double alpha = calculateThresholdMultiplier(usedCells);
            double threshold = noise * alpha;
            bool isTarget = profile[i] > threshold;

            file << i << ","
                 << profile[i] << ","
                 << noise << ","
                 << threshold << ","
                 << (isTarget ? 1 : 0)
                 << "\n";
        }

        file.close();

        std::cout << "\nCSV file created: " << fileName << "\n";
    }

private:
    int m_trainingCells;
    int m_guardCells;
    double m_pfa;

    std::pair<double, int> calculateNoiseAndCount(const std::vector<double>& profile, int testIndex)
    {
        double sum = 0.0;
        int count = 0;
        int size = static_cast<int>(profile.size());

        int startLeft = testIndex - m_guardCells - m_trainingCells;
        int endLeft = testIndex - m_guardCells - 1;

        for (int i = startLeft; i <= endLeft; i++)
        {
            if (i >= 0 && i < size)
            {
                sum += profile[i];
                count++;
            }
        }

        int startRight = testIndex + m_guardCells + 1;
        int endRight = testIndex + m_guardCells + m_trainingCells;

        for (int i = startRight; i <= endRight; i++)
        {
            if (i >= 0 && i < size)
            {
                sum += profile[i];
                count++;
            }
        }

        if (count == 0)
        {
            return {0.0, 0};
        }

        return {sum / count, count};
    }

    double calculateThresholdMultiplier(int usedCells)
    {
        if (usedCells <= 0)
        {
            return 0.0;
        }

        return usedCells * (std::pow(m_pfa, -1.0 / usedCells) - 1.0);
    }

    void printResult(int index,
                     double value,
                     double noise,
                     double threshold,
                     bool isTarget)
    {
        std::cout << index
                  << "\tvalue=" << value
                  << "\tnoise=" << noise
                  << "\tthreshold=" << threshold;

        if (isTarget)
        {
            std::cout << "\t*** TARGET ***";
        }

        std::cout << "\n";
    }
};

int main()
{
    int trainingCells = 8;
    int guardCells = 2;
    double pfa = 1e-4;

    ProfileGenerator generator;
    std::vector<double> profile = generator.generate(64);

    CfarDetector detector(trainingCells, guardCells, pfa);

    detector.process(profile);
    detector.exportToCsv(profile, "cfar_results.csv");

    return 0;
}
