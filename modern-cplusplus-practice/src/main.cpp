#include <variant>
#include <vector>
#include <iostream>

class Circle {
public:
    Circle(double radius) : radius(radius) {}
    double radius;
};

class Square {
public:
    Square(double side) : side(side) {}
    double side;
};

struct Draw {
    void operator()(Circle const& c) const {
        std::cout << "Draw circle with radius " << c.radius << std::endl;
    }

    void operator()(Square const& s) const {
        std::cout << "Draw square with side " << s.side << std::endl;
    }
};

using Shape = std::variant<Circle, Square>;
using Shapes = std::vector<Shape>;

void drawAllShapes(Shapes const& shapes) {
    for (auto const& shape : shapes) {
        std::visit(Draw{}, shape);
    }
}

// Visitor pattern using std::variant not old visitor pattern
int main() {
    Shapes shapes;
    shapes.emplace_back(Circle{1.0});
    shapes.emplace_back(Square{2.0});
    shapes.emplace_back(Circle{3.0});
    shapes.emplace_back(Square{4.0});
    drawAllShapes(shapes);

    return EXIT_SUCCESS;
}