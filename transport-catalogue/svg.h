#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg
{
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
            return { out, indent_step, indent + indent_step };
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

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object
    {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };


    class ObjectContainer
    {
    public:
        template <class Object>
        void Add(Object obj)
        {
            AddPtr(std::make_unique<Object>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        virtual ~ObjectContainer() = default;
    };


    class Drawable
    {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer
    {
    public:
        /*
         Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
         Пример использования:
         Document doc;
         doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
        */

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj);

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;
    private:
        std::vector<std::unique_ptr<Object>> objects_;
        // Прочие методы и данные, необходимые для реализации класса Document
    };

    struct Rgb
    {
        Rgb(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0)
            : red(r), green(g), blue(b)
        { }

        uint8_t red, green, blue;
    };

    struct Rgba
    {
        Rgba(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, double opacity = 1.0)
            : red(r), green(g), blue(b), opacity(opacity)
        { }

        uint8_t red, green, blue;
        double opacity;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{ "none" };

    struct PrintColorVisitor
    {
        std::ostream& out;
        void operator()(std::monostate) const
        {
            using namespace std::string_view_literals;
            out << "none"sv;
        }

        void operator()(const std::string& color) const
        {
            out << color;
        }

        void operator()(const Rgb& rgb) const
        {
            using namespace std::string_view_literals;
            out << "rgb("sv << +rgb.red << ',' << +rgb.green << ',' << +rgb.blue << ')';
        }

        void operator()(const Rgba& rgba) const
        {
            using namespace std::string_view_literals;
            out << "rgba("sv << +rgba.red << ',' << +rgba.green << ',' << +rgba.blue << ',' << rgba.opacity << ')';
        }
    };

    inline std::ostream& operator<<(std::ostream& out, const Color& color)
    {
        std::visit(PrintColorVisitor{ out }, color);
        return out;
    }

    enum class StrokeLineCap
    {
        BUTT, ROUND, SQUARE
    };

    enum class StrokeLineJoin
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND
    };

    inline std::ostream& operator<<(std::ostream& out, StrokeLineCap slc)
    {
        using namespace std::string_view_literals;
        switch (slc)
        {
        case svg::StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case svg::StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case svg::StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        default:
            break;
        }
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin slj)
    {
        using namespace std::string_view_literals;
        switch (slj)
        {
        case svg::StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case svg::StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case svg::StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case svg::StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case svg::StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        default:
            break;
        }

        return out;
    }

    template <class Owner>
    class PathProps
    {
    public:
        Owner& SetFillColor(Color fill_color)
        {
            fill_color_ = std::move(fill_color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color stroke_color)
        {
            stroke_color_ = std::move(stroke_color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width)
        {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap stroke_line_cap)
        {
            stroke_line_cap_ = stroke_line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin stroke_line_join)
        {
            stroke_line_join_ = stroke_line_join;
            return AsOwner();
        }

    protected:
        void RenderAttrs(std::ostream& out) const
        {
            using namespace std::literals;

            if (fill_color_)
                out << " fill=\""sv << *fill_color_ << "\""sv;

            if (stroke_color_)
                out << " stroke=\""sv << *stroke_color_ << "\""sv;

            if (stroke_width_)
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;

            if (stroke_line_cap_)
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;

            if (stroke_line_join_)
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
        }
    private:
        Owner& AsOwner()
        {
            return static_cast<Owner&>(*this);
        }
    private:
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    /*
    * Класс Circle моделирует элемент <circle> для отображения круга
    * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
    */
    class Circle final : public Object, public PathProps<Circle>
    {
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
    class Polyline final : public Object, public PathProps<Polyline>
    {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        void RenderObject(const RenderContext& context) const override;
    private:
        std::vector<Point> points_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text>
    {
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

        // Прочие данные и методы, необходимые для реализации элемента <text>
    private:
        void RenderObject(const RenderContext& context) const override;
    private:
        Point position_;
        Point offset_;
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

}  // namespace svg