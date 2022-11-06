#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<< (std::ostream& out, const svg::Rgb& color){
    out << "rgb("sv << color.red << ","sv << color.green << ","sv << color.blue << ")"sv;
    return out;
}

std::ostream& operator<< (std::ostream& out, const svg::Rgba& color){
    out << "rgba("sv << color.red << ","sv << color.green << ","sv << color.blue << ","sv << color.opacity << ")"sv;
    return out;
}

struct ColorPrinter{
    std::ostream& out;
    void operator() (std::monostate) const{
        out << "none";
    }
    void operator() (std::string color) const{
        out << color;
    }
    void operator() (svg::Rgb color) const{
        out << color;
    }
    void operator() (svg::Rgba color) const{
        out << color;
    }
};


std::ostream& operator<< (std::ostream& out, const svg::Color& color){
    std::visit(ColorPrinter{out}, color);
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\""sv;
    out << " r=\""sv << radius_ << "\""sv;
    //вывод атрибутов унаследованных от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point){
    polyline_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv ;
    bool is_first = true;
    for (const auto& point : polyline_){
        if(!is_first){
            out << " "sv;
        }
        out << point.x << ","sv << point.y;
        is_first = false;
    }
    out << "\""sv;
    //вывод атрибутов унаследованных от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos){
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size){
    font_size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family){
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data){
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const{
    auto& out = context.out;
    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    if (font_size_ != 0){
        out << " font-size=\""sv << font_size_ << "\""sv;
    }
    if (!font_family_.empty()){
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()){
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    //вывод атрибутов унаследованных от PathProps
    RenderAttrs(context.out);
    out << ">"sv;
    for(const auto& c : data_){
        if (c == '\"'){
            out << "&quot;"sv;
        } else if (c == '\''){
            out << "&apos;"sv;
        } else if (c == '<'){
            out << "&lt;"sv;
        } else if (c == '>'){
            out << "&gt;"sv;
        } else if (c == '&'){
            out << "&amp;"sv;
        } else {
            out << c;
        }

    }
    out << "</text>"sv;
}

// ---------- Document ------------------
void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.emplace_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << "\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << "\n";
    RenderContext context(out, 2, 2);
    for(const auto& obj : objects_){
        obj->Render(context);
    }
    out << "</svg>"sv << "\n";
}

}  // namespace svg
