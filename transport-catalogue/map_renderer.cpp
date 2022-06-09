#include "map_renderer.h"

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



    void MapRenderer::SetSettings (RenderSettings& settings) {
        settings_ = settings;
    }

    void MapRenderer::SetBusNameToBus(std::map<std::string_view, domain::Bus*>& busname_to_bus) {
        busname_to_bus_ = busname_to_bus;
    }

    void MapRenderer::RenderBusRoute(const sphere_projector::SphereProjector& projector) {
        int color_index = 0;
        for (const auto& [busname, bus_ptr]: busname_to_bus_) {
            if (!bus_ptr->stop_names_.empty()) {
                svg::Polyline line;
                if (bus_ptr->circle) {
                    for (auto &stop: bus_ptr->stop_names_) {
                        line.AddPoint(projector(stop->coord_));
                    }
                } else {
                    for (size_t i = 0; i < bus_ptr->stop_names_.size(); ++i) {
                        line.AddPoint(projector(bus_ptr->stop_names_[i]->coord_));
                    }
                    for (size_t i = bus_ptr->stop_names_.size() - 1; i > 0; --i) {
                        line.AddPoint(projector(bus_ptr->stop_names_[i - 1]->coord_));
                    }
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
    }

    void MapRenderer::RenderBusNumberText(const std::string& name, geo::Coordinates& coord,
                                          int& color_index,
                                          const sphere_projector::SphereProjector& projector,
                                          svg::Text& background_text,
                                          svg::Text& text) {

        background_text.SetFontFamily("Verdana"s)
                .SetOffset({ settings_.bus_label_offset[0], settings_.bus_label_offset[1]})
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontWeight("bold"s)
                .SetStrokeColor(settings_.underlayer_color)
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetData(std::string(name))
                .SetPosition(projector(coord));
        document_.Add(std::move(background_text));
        text.SetFontFamily("Verdana"s)
                .SetOffset({ settings_.bus_label_offset[0], settings_.bus_label_offset[1]})
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontWeight("bold"s)
                .SetFillColor(settings_.color_palette[color_index % settings_.color_palette.size()])
                .SetData(name)
                .SetPosition(projector(coord));
        document_.Add(std::move(text));
    }

    void MapRenderer::RenderBusNumber(const sphere_projector::SphereProjector& projector) {
        int color_index = 0;
        svg::Text background_text, text;
        for (const auto & [busname, bus_ptr]: busname_to_bus_) {
            if (!bus_ptr->stop_names_.empty()) {
                if (bus_ptr->circle) {


                    RenderBusNumberText(std::string(busname),
                                        bus_ptr->stop_names_.front()->coord_,
                                        color_index,
                                        projector, background_text, text);
                    ++color_index;
                } else {

                    RenderBusNumberText(std::string(busname),
                                        bus_ptr->stop_names_.front()->coord_,
                                        color_index,
                                        projector, background_text, text);
                    if (bus_ptr->stop_names_.front()->name_ != bus_ptr->stop_names_.back()->name_) {
                        RenderBusNumberText(std::string(busname),
                                            bus_ptr->stop_names_.back()->coord_,
                                            color_index,
                                            projector, background_text, text);
                    }
                    ++color_index;
                }
            }
        }
    }

    void MapRenderer::StopToStopCoord() {
        for (const auto& [busname, bus_ptr] : busname_to_bus_) {
            for (const auto& stop : bus_ptr->stop_names_) {
                stop_to_stops_coord_[stop->name_] = &stop->coord_;
            }
        }
    }

    void MapRenderer::RenderStopsDot(const sphere_projector::SphereProjector& projector) {

        for (const auto& [stop, stop_coord] : stop_to_stops_coord_) {
            svg::Circle circle;
            circle.SetRadius(settings_.stop_radius)
                    .SetCenter(projector(*stop_coord))
                    .SetFillColor("white");
            document_.Add(std::move(circle));
        }
    }

    void MapRenderer::RenderStopNames(const sphere_projector::SphereProjector& projector) {

        for (const auto& [stop, stop_coord] : stop_to_stops_coord_) {
            svg::Text background_text, text;
            background_text.SetFontFamily("Verdana"s)
                    .SetOffset({ settings_.stop_label_offset[0], settings_.stop_label_offset[1] })
                    .SetFontSize(settings_.stop_label_font_size)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetFillColor(settings_.underlayer_color)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetData(std::string (stop))
                    .SetPosition(projector(*stop_coord));
            document_.Add(std::move(background_text));

            text.SetFontFamily("Verdana"s)
                    .SetOffset({ settings_.stop_label_offset[0], settings_.stop_label_offset[1] })
                    .SetFontSize(settings_.stop_label_font_size)
                    .SetFillColor("black")
                    .SetData(std::string(stop))
                    .SetPosition(projector(*stop_coord));
            document_.Add(std::move(text));
        }
    }



    void MapRenderer::Render(std::ostream& out) {
        std::vector<geo::Coordinates> coord;
        for (const auto& [busname, bus_ptr] : busname_to_bus_) {
            if (!bus_ptr->stop_names_.empty()) {
                for (const auto& stop : bus_ptr->stop_names_) {
                    coord.push_back(stop->coord_);
                }
            }
        }

        sphere_projector::SphereProjector projector(coord.begin(),
                                                    coord.end(),
                                                    settings_.width,
                                                    settings_.height,
                                                    settings_.padding);

        RenderBusRoute(projector);
        RenderBusNumber(projector);

        StopToStopCoord();
        RenderStopsDot(projector);
        RenderStopNames(projector);

        document_.Render(out);
    }

}