#include "movie.hpp"
#include <iostream>
#include <cmath>
#include <string>

std::string Movie::getID() const
{
    return this->name + "_" + this->year;
}

void Movie::print() const
{
    std::cout << this->name << std::endl;
    std::cout << '\t' << "Year: " << this->year << std::endl;
    std::cout << '\t' << "Genres: " << setToString(this->genres) << std::endl;
    std::cout << '\t' << "Available Languages: " << setToString(this->languages) << std::endl;
    std::cout << '\t' << "Directors: " << setToString(this->directors) << std::endl;
    std::cout << '\t' << "Notable Actors: " << setToString(this->actors) << std::endl;
    std::cout << '\t' << "Notable Writers: " << setToString(this->writers) << std::endl;
    std::cout << '\t' << "Average Rating: " << this->avgRating << std::endl;
    std::cout << '\t' << "Number of Ratings: " << this->ratings << std::endl;
}

std::string Movie::setToString(const std::set<std::string>& set) const
{
    std::string str; 

    for(auto it = set.begin(); it != set.end(); it++)
    {
        str += *it;
        if(it != --set.end())
            str += ", ";
    }

    return str;
}


void Movie::scoreMovies(std::map<std::string, Movie> movies, Movie* selectedMovie) const
{
	float k_r = 1;
	float k_y = -0.25;
	float k_g = 1;
	float k_a = 1.5;
	float k_w = 1;
	float k_d = 2;


	//goes through map of movies, and scores them based on input movie
	for (auto it = movies.begin(); it != movies.end(); it++) 
	{
		Movie* thisMovie = &it->second;

		float ratingScore = (thisMovie->avgRating);
		float yearScore = abs(stof(selectedMovie->year) - std::stof(thisMovie->year));
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

		float thisRecomendationScore = pow(ratingScore, k_r) * pow(yearScore, k_y) * pow(genreScore, k_g) * pow(actorScore, k_a) * pow(writerScore, k_w) * pow(directorScore, k_d);
		
		thisMovie->score = thisRecomendationScore;
	}
}
