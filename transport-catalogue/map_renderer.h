#pragma once

#include <map>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "svg.h"



namespace map_renderer {

    using namespace std::string_literals;

    namespace sphere_projector {

        inline const double EPSILON = 1e-6;
        bool IsZero(double value) ;

        class SphereProjector {
        public:
            // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
            template <typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                            double max_width, double max_height, double padding);

            // Проецирует широту и долготу в координаты внутри SVG-изображения
            svg::Point operator()(geo::Coordinates coords) const;

        private:
            double padding_;
            double min_lon_ = 0;
            double max_lat_ = 0;
            double zoom_coeff_ = 0;
        };
    }

    struct RenderSettings {
        using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
        double width = 0;
        double height = 0;
        double padding = 0;
        double line_width = 0;
        double stop_radius = 0;
        int bus_label_font_size = 0;
        double bus_label_offset[2] = { 0, 0 };
        int stop_label_font_size = 0;
        double stop_label_offset[2] = { 0, 0 };
        Color underlayer_color;
        double underlayer_width = 0;
        std::vector<Color> color_palette;
    };

    class MapRenderer {
    public:

        MapRenderer() = default;

        void SetSettings (RenderSettings& set);

        void SetRoutes(std::map<std::string_view, std::vector<geo::Coordinates>>& bus_to_stops_coord);

        void SetStops(std::map<std::string_view, geo::Coordinates>& stop_to_stop_coord);

        void SetBusNameToBus(std::map<std::string_view, domain::Bus*>& busname_to_bus);

        void BusRouteRender(sphere_projector::SphereProjector& projector);

        void BusNumberRender(sphere_projector::SphereProjector& projector);
        void BusNumberTextRender(const std::string& name, geo::Coordinates& coord,
                                 int& color_index,
                                 sphere_projector::SphereProjector& projector,
                                 svg::Text& background_text,
                                 svg::Text& text);

        void Render(std::ostream& out);

    private:
        RenderSettings settings_;
        std::map<std::string_view, std::vector<geo::Coordinates>> bus_to_stops_coord_;

        std::map<std::string_view, geo::Coordinates> stop_to_stops_coord_;
        std::map<std::string_view, domain::Bus*> busname_to_bus_;
        svg::Document document_;

    };

}