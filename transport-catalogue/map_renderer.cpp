#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
namespace map_renderer {
    namespace sphere_projector {

        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template<typename PointInputIt>
        SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                                         double max_width, double max_height, double padding)
                : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
            return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    }



    void MapRenderer::SetSettings (RenderSettings& set) {
        settings_ = set;
    }

    void MapRenderer::SetRoutes(std::map<std::string_view, std::vector<geo::Coordinates>>& bus_to_stops_coord) {
        bus_to_stops_coord_ = bus_to_stops_coord;
    }

    void MapRenderer::SetStops(std::map<std::string_view, geo::Coordinates>& stop_to_stop_coord) {
        stop_to_stops_coord_ = stop_to_stop_coord;
    }

    void MapRenderer::SetBusNameToBus(std::map<std::string_view, domain::Bus*>& busname_to_bus) {
        busname_to_bus_ = busname_to_bus;
    }

    void MapRenderer::BusRouteRender(sphere_projector::SphereProjector& projector) {
        int color_index = 0;
        for (const auto &x: bus_to_stops_coord_) {

            svg::Polyline line;
            for (auto& stop : x.second) {
                line.AddPoint(projector(stop));
            }
            line.SetFillColor("none");
            line.SetStrokeColor(settings_.color_palette[color_index % settings_.color_palette.size()]);
            line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            line.SetStrokeWidth(settings_.line_width);
            line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            color_index++;
            document_.Add(line);
        }
    }

    void MapRenderer::BusNumberTextRender(const std::string& name, geo::Coordinates& coord,
                                          int& color_index,
                                          sphere_projector::SphereProjector& projector,
                                          svg::Text& background_text,
                                          svg::Text& text) {

        background_text.SetFontFamily("Verdana"s)
                .SetOffset({ settings_.bus_label_offset[0], settings_.bus_label_offset[1] })
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontWeight("bold"s)
                .SetStrokeColor(settings_.underlayer_color)
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetData(std::string(name))
                .SetPosition(projector(coord));
        document_.Add(background_text);
        text.SetFontFamily("Verdana"s)
                .SetOffset({ settings_.bus_label_offset[0], settings_.bus_label_offset[1] })
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontWeight("bold"s)
                .SetFillColor(settings_.color_palette[color_index % settings_.color_palette.size()])
                .SetData(name)
                .SetPosition(projector(coord));
        document_.Add(text);
    }

    void MapRenderer::BusNumberRender(sphere_projector::SphereProjector& projector) {
        int color_index = 0;
        for (const auto &x: busname_to_bus_) {
            if (x.second->circle) {
                svg::Text background_text, text;

                BusNumberTextRender(std::string(x.first),
                                    x.second->stop_names_.front()->coord_,
                                    color_index,
                                    projector, background_text, text);
                ++color_index;
            } else {
                svg::Text background_text, text;

                BusNumberTextRender(std::string(x.first),
                                    x.second->stop_names_.front()->coord_,
                                    color_index,
                                    projector, background_text, text);
                BusNumberTextRender(std::string(x.first),
                                    x.second->stop_names_.back()->coord_,
                                    color_index,
                                    projector, background_text, text);
                ++color_index;
            }

        }
    }

    void MapRenderer::Render(std::ostream& out) {
        std::vector<geo::Coordinates> coord;
        for (const auto& x : bus_to_stops_coord_) {
            if (!x.second.empty()) {
                for (const auto& stop : x.second) {
                    coord.push_back(stop);
                }
            }
        }

        sphere_projector::SphereProjector projector(coord.begin(),
                                                    coord.end(),
                                                    settings_.width,
                                                    settings_.height,
                                                    settings_.padding);

        BusRouteRender(projector);
        BusNumberRender(projector);
        document_.Render(out);
    }

}