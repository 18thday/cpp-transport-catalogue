#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

struct Rgb{
    Rgb() = default;
    Rgb(int r, int g, int b)
    : red(r), green(g), blue(b){}
    uint16_t red = 0;
    uint16_t green = 0;
    uint16_t blue = 0;
};

std::ostream& operator<< (std::ostream& out, const svg::Rgb& color);

struct Rgba{
    Rgba() = default;
    Rgba(int r, int g, int b, double a)
    : red(r), green(g), blue(b), opacity(a){}
    uint16_t red = 0;
    uint16_t green = 0;
    uint16_t blue = 0;
    double opacity = 1.0;
};

std::ostream& operator<< (std::ostream& out, const svg::Rgba& color);

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
inline const Color NoneColor{};

std::ostream& operator<< (std::ostream& out, const svg::Color& color);

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

inline std::ostream& operator<< (std::ostream& out, const svg::StrokeLineCap& line_cap){
    using namespace std::literals;
    if (line_cap == StrokeLineCap::ROUND){
        out << "round"sv;
    } else if (line_cap == StrokeLineCap::SQUARE){
        out << "square"sv;
    } else {
        out << "butt"sv;
    }
    return out;
}

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

inline std::ostream& operator<< (std::ostream& out, const svg::StrokeLineJoin& line_cap){
    using namespace std::literals;
    if (line_cap == StrokeLineJoin::ARCS){
        out << "arcs"sv;
    } else if (line_cap == StrokeLineJoin::BEVEL){
        out << "bevel"sv;
    } else if (line_cap == StrokeLineJoin::MITER_CLIP){
        out << "miter-clip"sv;
    } else if (line_cap == StrokeLineJoin::ROUND){
        out << "round"sv;
    } else {
        out << "miter"sv;
    }
    return out;
}

template <typename Owner>
class PathProps{
public:
    Owner& SetFillColor(Color color){
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color){
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width){
        width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap){
        line_cap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
        line_join_ = line_join;
        return AsOwner();
    }
protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;
        if (fill_color_){
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_){
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (width_){
            out << " stroke-width=\""sv << *width_ << "\""sv;
        }
        if (line_cap_){
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
        if (line_join_){
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }
private:
    Owner& AsOwner(){
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> polyline_;

};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

    Point pos_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;

    // Прочие данные и методы, необходимые для реализации элемента <text>
};

class ObjectContainer{
public:
    template <typename Obj>
    void Add(Obj obj){
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
protected:
    ~ObjectContainer() = default;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

class Document : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
private:
    std::vector<std::unique_ptr<Object>> objects_;

};

}  // namespace svg
