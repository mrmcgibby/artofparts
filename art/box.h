#ifndef BOX_H
#define BOX_H

#include <array>
#include <string>
#include <vector>

static const size_t dimensions = 2;

class box {
public:
    typedef std::array<double, dimensions> point;

    box();
    box(double size, std::string name);

    std::string name() const;

    // calculate the box sizes for the current tree
    void map(point new_extent, point new_origin);
    // fix the child areas to match the parent
    void remap();
    void resize();
    // the size of the box and children should be kept up to date
    double size();

    void add(box b);
    template <typename... Args>
    void add(box b, Args... args) {
        add(b);
        add(args...);
    }
    void insert(int pos, box b);

    void fix_dimensions();

    double origin(int dimension = -1);
    double extent(int dimension = -1);

    box& child(size_t index);
    size_t children();

    int next_dimension(int change = 1) const;

    void clear();
    void split(size_t index);
    void balance(bool top = false);
    void append(box new_box);
    box subbox(size_t start, size_t end) const;
    void add_desired(point desired);

    point center();
    double distance(point a, point b);
    double score();
    bool contains(point p) const;
    bool contains_desired();
    box::point desired() const;
    bool leaf();

    std::vector<box> flat();

private:
    std::vector<box> m_children;
    double m_size;
    point m_origin, m_extent;
    size_t m_dimension;
    std::string m_name;

    std::vector<point> m_desired;
};

#endif // BOX_H
