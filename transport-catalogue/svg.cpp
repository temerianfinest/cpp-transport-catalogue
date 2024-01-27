#include "svg.h"
#include <sstream>

namespace svg
{
    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i)
        {
            out << points_[i].x << ","sv << points_[i].y;

            if (i != points_.size() - 1)
                out << " "sv;
        }
        out << "\"";
        RenderAttrs(out);
        out << "/>"sv;
    }

    Text& Text::SetPosition(Point pos)
    {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        std::stringstream escaped;

        for (const auto& c : data)
        {
            switch (c)
            {
            case '&': escaped << "&amp;"; break;
            case '\'': escaped << "&apos;"; break;
            case '"': escaped << "&quot;"; break;
            case '<': escaped << "&lt;"; break;
            case '>': escaped << "&gt;"; break;
            default: escaped << c; break;
            }
        }

        data_ = escaped.str();
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\""sv;

        if (!font_family_.empty())
            out << " font-family=\""sv << font_family_ << "\""sv;
        if (!font_weight_.empty())
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        out << ">"sv << data_ << "</text>"sv;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj)
    {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        for (const auto& object : objects_)
            object->Render(out);

        out << "</svg>"sv;
    }

}  // namespace svg