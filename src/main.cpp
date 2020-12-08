#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <chrono>
#include <algorithm>
#include <locale>
#include "movie.hpp"
#include <math.h>

const std::string TITLE_AKAS = "datasets/title.akas.tsv";
const std::string TITLE_BASICS = "datasets/title.basics.tsv";
const std::string TITLE_RATINGS = "datasets/title.ratings.tsv";
const std::string TITLE_PRINCIPALS = "datasets/title.principals.tsv";
const std::string NAME_BASICS = "datasets/name.basics.tsv";
const std::string MASTER_FILE = "datasets/master.tsv";

bool LoadMovieBasics(std::map<std::string, Movie *> &moviesByName, std::map<std::string, Movie *> &moviesById)
{
    std::ifstream file(TITLE_BASICS);
    if (!file.is_open())
        return false;

    // Skip the first line
    std::string line;
    std::getline(file, line);
    // File headings:
    // tconst(0) titleType(1) primaryTitle(2) originalTitle(3) isAdult(4) startYear(5) endYear(6) runtimeMinutes(7) genres(8)
    while (std::getline(file, line))
    {
        std::istringstream buffer(line);
        std::string temp;
        std::vector<std::string> values;

        while (std::getline(buffer, temp, '\t'))
            values.push_back(temp);

        // Don't add this entry if it isn't a movie or is an adult movie
        if (values[1] != "movie" && values[1] != "tvMovie")
            continue;
        if (values[4] == "1")
            continue;

        Movie *movie = new Movie();
        movie->movieId = values[0];
        movie->name = values[2];

        std::istringstream genreBuffer(values[8]);
        std::string genre;
        while (std::getline(genreBuffer, genre, ','))
            if (genre != "\\N")
                movie->genres.insert(genre);

        if (values[5] == "\\N")
            movie->year = "UNKNOWN";
        else
            movie->year = values[5];

        if (moviesByName.count(movie->getID()) != 0)
        {
            //std::cout << "Potential conflict with title: " << movie->getID() << std::endl;

            // Remove the old duplicate from the map
            std::string oldId = moviesByName[movie->getID()]->movieId;
            delete moviesById[oldId];
            moviesById.erase(oldId);
        }

        moviesByName[movie->getID()] = movie;
        moviesById[movie->movieId] = movie;
    }

    return true;
}

bool LoadRatings(std::map<std::string, Movie *> &moviesById)
{
    std::ifstream file(TITLE_RATINGS);
    if (!file.is_open())
        return false;

    // Skip the first line
    std::string line;
    std::getline(file, line);
    // File headings:
    // tconst(0) averageRating(1) numVotes(2)
    while (std::getline(file, line))
    {
        std::istringstream buffer(line);
        std::string temp;
        std::vector<std::string> values;

        while (std::getline(buffer, temp, '\t'))
            values.push_back(temp);

        // Skip over movies not already added to the map
        if (moviesById.count(values[0]) == 0)
            continue;

        moviesById[values[0]]->avgRating = std::stof(values[1]);
        moviesById[values[0]]->ratings = std::stoi(values[2]);
    }

    return true;
}

bool LoadCastIds(std::map<std::string, Movie *> &moviesById)
{
    std::ifstream file(TITLE_PRINCIPALS);
    if (!file.is_open())
        return false;

    // Skip the first line
    std::string line;
    std::getline(file, line);
    // File headings:
    // tconst(0) ordering(1) nconst(2) category(3) job(4) characters(5)
    while (std::getline(file, line))
    {
        std::istringstream buffer(line);
        std::string temp;
        std::vector<std::string> values;

        while (std::getline(buffer, temp, '\t'))
            values.push_back(temp);

        // Skip over movies not already added to the map
        if (moviesById.count(values[0]) == 0)
            continue;

        //std::cout << values[0] << " : " << values[3] << std::endl;

        if (values[3] == "actor" || values[3] == "actress")
            moviesById[values[0]]->actorIds.insert(values[2]);
        else if (values[3] == "director")
            moviesById[values[0]]->directorIds.insert(values[2]);
        else if (values[3] == "writer")
            moviesById[values[0]]->writerIds.insert(values[2]);
    }

    return true;
}

bool LoadCastNames(std::map<std::string, Movie *> &moviesById)
{
    std::ifstream file(NAME_BASICS);
    if (!file.is_open())
        return false;

    // nconst:name
    std::map<std::string, std::string> cast;
    // Skip the first line
    std::string line;
    std::getline(file, line);
    // File headings:
    // nconst(0) primaryName(1) birthYear(2) deathYear(3) primaryProfession(4) knownForTitles(5)
    while (std::getline(file, line))
    {
        std::istringstream buffer(line);
        std::string temp;
        std::vector<std::string> values;

        // Store the first value, the ncount, to use as a key
        std::getline(buffer, temp, '\t');
        std::string nconst = temp;

        std::getline(buffer, temp, '\t');
        std::string name = temp;

        cast[nconst] = name;
    }

    for (auto movie : moviesById)
    {
        for (auto id : movie.second->actorIds)
            movie.second->actors.insert(cast[id]);

        for (auto id : movie.second->directorIds)
            movie.second->directors.insert(cast[id]);

        for (auto id : movie.second->writerIds)
            movie.second->writers.insert(cast[id]);
    }

    return true;
}
bool LoadLanguages(std::map<std::string, Movie *> &moviesById)
{
    std::ifstream file(TITLE_AKAS);
    if (!file.is_open())
        return false;

    // nconst:name
    std::map<std::string, std::string> cast;
    // Skip the first line
    std::string line;
    std::getline(file, line);
    // File headings:
    // titleId(0) ordering(1) title(2) region(3) language(4) types(5) attributes(6) isOriginalTitle(7)
    while (std::getline(file, line))
    {
        std::istringstream buffer(line);
        std::string temp;
        std::vector<std::string> values;

        std::getline(buffer, temp, '\t');
        std::string titleId = temp;

        if (moviesById.count(titleId) == 0)
            continue;

        // Only iterate 3 times so we can get the language
        for (int i = 0; i < 3; i++)
            std::getline(buffer, temp, '\t');

        if (temp != "\\N")
            moviesById[titleId]->languages.insert(temp);
    }

    return true;
}

std::string setToString(const std::set<std::string> &set)
{
    std::string str;

    if (set.empty())
        return "\\N";

    for (auto it = set.begin(); it != set.end(); it++)
    {
        str += *it;
        if (it != --set.end())
            str += ",";
    }

    return str;
}

void MoviesToTsv(std::map<std::string, Movie *> moviesById)
{
    std::ofstream ofs;
    ofs.open("datasets/master.tsv", std::ofstream::out | std::ofstream::trunc);

    ofs << "tconst	name	year	avgrating	ratings	genres	languages	directors	actors	writers	directorids	actorids	writerids" << std::endl;
    for (auto movie : moviesById)
    {
        ofs << movie.second->movieId << '\t';
        ofs << movie.second->name << '\t';
        ofs << movie.second->year << '\t';
        ofs << movie.second->avgRating << '\t';
        ofs << movie.second->ratings << '\t';

        ofs << setToString(movie.second->genres) << '\t';
        ofs << setToString(movie.second->languages) << '\t';
        ofs << setToString(movie.second->directors) << '\t';
        ofs << setToString(movie.second->actors) << '\t';
        ofs << setToString(movie.second->writers) << '\t';
        ofs << setToString(movie.second->directorIds) << '\t';
        ofs << setToString(movie.second->actorIds) << '\t';
        ofs << setToString(movie.second->writerIds) << '\t';

        ofs << std::endl;
    }

    ofs.close();
}

bool LoadMasterFile(std::map<std::string, Movie *> &moviesByName, std::map<std::string, Movie *> &moviesById)
{
    std::ifstream file(MASTER_FILE);
    if (!file.is_open())
        return false;

    std::cout << "Master file found, loading from master file..." << std::endl;

    // Skip the first line
    std::string line;
    std::getline(file, line);
    // File headings:
    // tconst(0) name(1) year(2) avgrating(3) ratings(4) genres(5) languages(6) directors(7) actors(8) writers(9) directorids(10) actorids(11) writerids(12)
    while (std::getline(file, line))
    {
        std::istringstream buffer(line);
        std::string temp;
        std::vector<std::string> values;

        while (std::getline(buffer, temp, '\t'))
            values.push_back(temp);

        Movie *movie = new Movie();
        movie->movieId = values[0];
        movie->name = values[1];
        movie->year = values[2];
        movie->avgRating = stof(values[3]);
        movie->ratings = stoi(values[4]);

        for (int i = 5; i < 13; i++)
        {
            std::istringstream buffer(values[i]);
            std::string bufferStr;
            while (std::getline(buffer, bufferStr, ','))
            {
                if (bufferStr != "\\N")
                {
                    switch (i)
                    {
                    case 5:
                        movie->genres.insert(bufferStr);
                        break;
                    case 6:
                        movie->languages.insert(bufferStr);
                        break;
                    case 7:
                        movie->directors.insert(bufferStr);
                        break;
                    case 8:
                        movie->actors.insert(bufferStr);
                        break;
                    case 9:
                        movie->writers.insert(bufferStr);
                        break;
                    case 10:
                        movie->directorIds.insert(bufferStr);
                        break;
                    case 11:
                        movie->actorIds.insert(bufferStr);
                        break;
                    case 12:
                        movie->writerIds.insert(bufferStr);
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        if (moviesByName.count(movie->getID()) != 0)
        {
            std::cout << "Potential conflict with title: " << movie->getID() << std::endl;

            // Remove the old duplicate from the map
            std::string oldId = moviesByName[movie->getID()]->movieId;
            delete moviesById[oldId];
            moviesById.erase(oldId);
        }

        moviesByName[movie->getID()] = movie;
        moviesById[movie->movieId] = movie;
    }

    return true;
}

std::vector <Movie*> selectionSort(int loops, std::map<std::string, Movie*> &moviesByName) //Cut down to only correct number of values
{
    int score = 0;
    int count = loops;

    std::vector<Movie*> sorted;

    if (count > moviesByName.size() or count < 0)
    {
        count = moviesByName.size();
    }
	else
	{
		count = count + 1;
	}


    for (auto it = moviesByName.begin(); it != moviesByName.end(); it++)
    {
        sorted.push_back(it->second);
    }

    int index = 0;
    for (int i = 0; i < count; i++)
    {
        for (int x = i; x < sorted.size(); x++)
        {
            if (sorted[index]->score < sorted[x]->score)
            {
                index = x;
            }
        }

        Movie *temp = sorted[i];
        sorted[i] = sorted[index];
        sorted[index] = temp;
    }
    return sorted; 
}   

void heapifyDown(std::vector<Movie*> &theHeap, int index, int size)
{
    int leftChild = 2 * index + 1;
    int rightChild = 2 * index + 2;
    int biggest = index;

    if (leftChild < size && theHeap[leftChild]->score > theHeap[biggest]->score)
    {
        biggest = leftChild;
    }

    if (rightChild < size && theHeap[rightChild]->score > theHeap[biggest]->score)
    {
        biggest = rightChild;
    }

    if (biggest != index)
    {
        Movie* temp = theHeap[index];
        theHeap[index] = theHeap[biggest];
        theHeap[biggest] = temp;

        heapifyDown(theHeap, biggest, size);
    }
}

std::vector <Movie*> heapSort(int loops, std::map<std::string, Movie*> &moviesByName)
{
    std::vector <Movie*> sorted;
    std::vector <Movie*> output;


    for (auto it = moviesByName.begin(); it != moviesByName.end(); it++)
    {
        sorted.push_back(it->second);
    }

    for (int i = sorted.size() / 2 - 1; 0 <= i; i--)
    {
        heapifyDown(sorted, i, sorted.size());
    }

    for (int i = 0; i < loops; i++)
    {
        output.push_back(sorted[0]);
        sorted[0] = sorted[sorted.size() - 1];
        sorted.resize(sorted.size() - 1);
        heapifyDown(sorted, 0, sorted.size());
    }

    return output;
}

std::vector< std::pair<int, Movie*>> suggestAlt(std::string title, std::map<std::string, Movie*>& movies)
{
    std::vector< std::pair<int, Movie*>> suggestions;

    for (auto it = movies.begin(); it != movies.end(); it++)
    {
        if (it->second->name.find(title) != std::string::npos)
        {
            suggestions.push_back(std::make_pair(it->second->ratings, it->second));
        }
    }
    return suggestions;
}

Movie* findMovie(std::string title, std::string year, std::map<std::string, Movie*> &movies) //Todo account for a null search result
{
    std::string key = title + "_" + year;

    auto it = movies.find(key);

    Movie* output = NULL;

    if (it != movies.end())
    {
        output = it->second;
    }

    else
    {
        std::cout << "Movie: " << title << " not found!" << std::endl << std::endl;
        std::vector< std::pair<int, Movie*>> suggestions = suggestAlt(title, movies);
        sort(suggestions.begin(), suggestions.end(), std::greater <std::pair<int, Movie*>>());

        if (suggestions.size() > 0)
        {
            std::cout << "Did you mean?" << std::endl;

            int loops = 0;
            if (suggestions.size() < 10)
            {
                loops = suggestions.size();
            }
            
            else
            {
                loops = 10;
            }
            for (int i = 0; i < loops; i++)
            {
                std::cout << i + 1 << ". " << "\"" << suggestions[i].second->name << "\", "<< suggestions[i].second->year << std::endl;
            }

            int x;
            std::cout << "Pick an option: ";
            std::cin >> x;
            output = suggestions[x - 1].second;
        }

        else
        {
            std::cout << "No results" << std::endl;
        }
    }

    return output; 
}

void scoreMovies(std::map<std::string, Movie*> movies, Movie* selectedMovie)
{
	float k_rs = 1;
	float k_ra = 1;
	float k_y = -0.25;
	float k_g = 1.5;
	float k_a = 1.5;
	float k_w = 1;
	float k_d = 1.6;


	//goes through map of movies, and scores them based on input movie
	for (auto it = movies.begin(); it != movies.end(); it++)
	{
		Movie* thisMovie = it->second;

		float ratingsScore = 1 - abs((float(thisMovie->ratings) - float(selectedMovie->ratings))) / (float(thisMovie->ratings) + float(selectedMovie->ratings) + float(1));
		float ratingScore = (thisMovie->avgRating);
		float yearScore = abs(stof(selectedMovie->year) - stof(thisMovie->year)) + 1;
		float genreScore = 0;
		float actorScore = 1;
		float writerScore = 1;
		float directorScore = 1;

		for (auto it = thisMovie->genres.begin(); it != thisMovie->genres.end(); it++) {
			genreScore += selectedMovie->genres.count(*it);
		}
		for (auto it = thisMovie->actorIds.begin(); it != thisMovie->actorIds.end(); it++) {
			actorScore += selectedMovie->actorIds.count(*it);
		}
		for (auto it = thisMovie->writerIds.begin(); it != thisMovie->writerIds.end(); it++) {
			writerScore += selectedMovie->writerIds.count(*it);
		}
		for (auto it = thisMovie->directorIds.begin(); it != thisMovie->directorIds.end(); it++) {
			directorScore += selectedMovie->directorIds.count(*it);
		}

		float thisRecomendationScore = pow(ratingsScore, k_rs) * pow(ratingScore, k_ra) * pow(yearScore, k_y) * pow(genreScore, k_g) * pow(actorScore, k_a) * pow(writerScore, k_w) * pow(directorScore, k_d);

		thisMovie->score = thisRecomendationScore;
	}
}

int main()
{
	// Key: NAME_YEAR
	std::map<std::string, Movie*> moviesByName;

	// Stores a ID:NAME pair, so we can have fast lookups by both names and ID
	std::map<std::string, Movie*> moviesById;

	bool masterFileUsed = false;

	auto startTime = std::chrono::high_resolution_clock::now();

	if (masterFileUsed = LoadMasterFile(moviesByName, moviesById))
		goto finished_loading;

	std::cout << "Loading database..." << std::endl;
	if (!LoadMovieBasics(moviesByName, moviesById))
	{
		std::cout << "Missing file: " << TITLE_BASICS << std::endl;
		return 1;
	}

	std::cout << "Loading ratings..." << std::endl;
	if (!LoadRatings(moviesById))
	{
		std::cout << "Missing file: " << TITLE_RATINGS << std::endl;
		return 1;
	}

	std::cout << "Loading cast..." << std::endl;
	if (!LoadCastIds(moviesById))
	{
		std::cout << "Missing file: " << TITLE_PRINCIPALS << std::endl;
		return 1;
	}

	std::cout << "Processing names..." << std::endl;
	if (!LoadCastNames(moviesById))
	{
		std::cout << "Missing file: " << NAME_BASICS << std::endl;
		return 1;
	}

	std::cout << "Loading languages..." << std::endl;
	if (!LoadLanguages(moviesById))
	{
		std::cout << "Missing file: " << TITLE_AKAS << std::endl;
		return 1;
	}

finished_loading:
	auto stopTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);
	std::cout << "Time elasped: " << duration.count() << "ms" << std::endl;

	if (!masterFileUsed)
	{
		std::cout << "A master file can be created for faster loading times" << std::endl;
		std::cout << "Would you like to create a master file? (y/n): ";

		std::string answer;

		std::cin >> answer;

		if ((unsigned char)std::tolower(answer[0]) == 'y')
		{
			std::cout << "Saving to master file..." << std::endl;
			MoviesToTsv(moviesById);
		}

	}

	while (true)
	{
		std::string title;
		std::string year;
		std::string algoSelction;

		std::cout << "Enter a Movie Name: ";
		getline(std::cin, title);
		std::cout << "Enter the year published (type UNKNOWN if the movie has no known date): ";
		getline(std::cin, year);

		bool selectionSortChosen;
		bool validAlgoChosen = false;
		while (not validAlgoChosen) {
			std::cout << "Which Sorting algorithm would you like to use: " << std::endl;
			std::cout << "1) selection sort " << std::endl;
			std::cout << "2) heap sort " << std::endl;
			getline(std::cin, algoSelction);

			if (algoSelction == "1")
			{
				selectionSortChosen = true;
				validAlgoChosen = true;
			}
			else if (algoSelction == "2")
			{
				selectionSortChosen = false;
				validAlgoChosen = true;
			}
		}

		//Not needed with after search suggestion function
		//std::string key = title + "_" + year; 
		//if (moviesByName.count(key) == 0)
		//{
		//	std::cout << "Movie not found!" << std::endl;
		//	return 2;
		//}

		Movie* selectedMovie = findMovie(title, year, moviesByName);

		selectedMovie->print();
		std::cout << std::endl;

		std::string imdbId = selectedMovie->movieId;

		scoreMovies(moviesByName, selectedMovie);

		if (selectionSortChosen) 
		{
			std::cout << "Top 10 Recommendations: selection sort" << std::endl;

			auto startTime = std::chrono::high_resolution_clock::now();

			auto bsorted = selectionSort(10, moviesByName);

			auto stopTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);

			int bshift = 0;
			if (bsorted[0]->name == selectedMovie->name) {
				bshift = 1;
			}
			for (int i = 0 + bshift; i < 10 + bshift; i++)
			{
				std::cout << i << "-------------------------" << std::endl;
				bsorted[i]->print();
				std::cout << "\tRecomendation Score: " << bsorted[i]->score << std::endl;
			}
			std::cout << std::endl;
			std::cout << "Time elasped: " << duration.count() << "ms" << std::endl;
			std::cout << std::endl;
		}
		else if (!selectionSortChosen)
		{
			std::cout << "Top 10 Recommendations: heapsort sort" << std::endl;

			auto startTime = std::chrono::high_resolution_clock::now();
			
			auto hsorted = heapSort(10 + 1, moviesByName);
			
			auto stopTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);

			int hshift = 0;
			if (hsorted[0]->name == selectedMovie->name) {
				hshift = 1;
			}
			for (int i = 0 + hshift; i < 10 + hshift; i++)
			{
				std::cout << i << "-------------------------" << std::endl;
				hsorted[i]->print();
				std::cout << "\tRecomendation Score: " << hsorted[i]->score << std::endl;
				std::cout << std::endl;
			}
			title = "";
			year = "";

			std::cout << std::endl;
			std::cout << "Time elasped: " << duration.count() << "ms" << std::endl;
			std::cout << std::endl;
		}
	}

	return 0;
}
