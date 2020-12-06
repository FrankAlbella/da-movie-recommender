#include <string>
#include <set>
#include <map>
#include <vector>

class Movie
{
private:
    std::string setToString(const std::set<std::string>& set) const; 
public:
    std::string movieId;
    std::string name;
    std::string year;

    float avgRating;
    int ratings; 
    
    std::set<std::string> genres;
    std::set<std::string> languages;
    std::set<std::string> directors;
    std::set<std::string> actors;
    std::set<std::string> writers;

    std::set<std::string> directorIds;
    std::set<std::string> actorIds;
    std::set<std::string> writerIds;
    float score = -1; 

    std::vector<Movie> getRecommendations(std::map<std::string, Movie> movies) const;
    void scoreMovies(std::map<std::string, Movie> movies, Movie* selectedMovie) const;
    void print() const;
    // Returns the ID used as the key to this movie in an std::map
    std::string getID() const;
};
