#include "serialization.h"

namespace transport_data_base {

    void TransportCatalogueSerialization::SetSettings(const SerializationSettings& settings) {
        settings_ = settings;
    }

    void TransportCatalogueSerialization::Serialize(){

        *base_.mutable_transport_catalogue() = std::move(SerializeCatalogue());
        *base_.mutable_map_renderer() = std::move(SerializeMapRenderer());

        std::filesystem::path path = settings_.file_name;
        std::ofstream out_file(path, std::ios::binary);
        base_.SerializeToOstream(&out_file);
    }

    transport_data_base::Catalogue TransportCatalogueSerialization::SerializeCatalogue() {
        Catalogue tmp_catalogue;
        int s = 0;
        for (auto& [stop_name, stop_data] : catalogue_.GetStopNameToStop()) {
            tmp_catalogue.add_stops();
            *tmp_catalogue.mutable_stops(s) = std::move(SerializeStop(stop_data));
            ++s;
        }
        int b = 0; // bus index in proto
        for (auto& [bus_name, bus_data] : catalogue_.GetBusesForRender()) {
            tmp_catalogue.add_buses();
            *tmp_catalogue.mutable_buses(b) = std::move(SerializeBus(bus_data));
            ++b;
        }
        int d = 0; // distance counter
        for (auto& [stops, length] : catalogue_.GetDistanceTable()) {
            tmp_catalogue.add_distances();          // add element in array of proto
            *tmp_catalogue.mutable_distances(d) = std::move(SerializeDistance(stops, length)); // modify new element
            ++d;
        }
        return tmp_catalogue;
    }

    transport_data_base::Stop TransportCatalogueSerialization::SerializeStop(const domain::Stop* stop_data) {
        transport_data_base::Stop tmp_stop;
        tmp_stop.set_name(std::string(stop_data->name_));
        tmp_stop.mutable_coord()->set_lat(stop_data->coord_.lat);
        tmp_stop.mutable_coord()->set_lng(stop_data->coord_.lng);
        return tmp_stop;
    }

    transport_data_base::Bus TransportCatalogueSerialization::SerializeBus(const domain::Bus* bus_data) {
        transport_data_base::Bus tmp_bus;
        tmp_bus.set_number(std::string(bus_data->number_));
        for (const auto stop : bus_data->stop_names_) {
            tmp_bus.add_stop_names(stop->name_);
        }
        tmp_bus.set_circle(bus_data->circle);
        return tmp_bus;
    }

    transport_data_base::Distance TransportCatalogueSerialization::SerializeDistance(const std::pair<domain::Stop*, domain::Stop*> stops,
                                                                                     size_t length) {
        transport_data_base::Distance tmp_dist;
        tmp_dist.set_from(stops.first->name_);
        tmp_dist.set_to(stops.second->name_);
        tmp_dist.set_distance(static_cast<uint32_t>(length));
        return tmp_dist;
    }

    transport_data_base::MapRenderer TransportCatalogueSerialization::SerializeMapRenderer() {
        transport_data_base::MapRenderer tmp_map;
        *tmp_map.mutable_settings() = std::move(SerializeRenderSettings());
        return tmp_map;
    }

    transport_data_base::RenderSettings TransportCatalogueSerialization::SerializeRenderSettings() {
        transport_data_base::RenderSettings tmp_render_settings;
        map_renderer::RenderSettings cat_rend_set = renderer_.GetRenderSettings();

        tmp_render_settings.set_width(cat_rend_set.width);
        tmp_render_settings.set_height(cat_rend_set.height);
        tmp_render_settings.set_padding(cat_rend_set.padding);
        tmp_render_settings.set_line_width(cat_rend_set.line_width);
        tmp_render_settings.set_stop_radius(cat_rend_set.stop_radius);
        tmp_render_settings.set_bus_label_font_size(cat_rend_set.bus_label_font_size);
        tmp_render_settings.add_bus_label_offset(cat_rend_set.bus_label_offset[0]);
        tmp_render_settings.add_bus_label_offset(cat_rend_set.bus_label_offset[1]);
        tmp_render_settings.set_stop_label_font_size(cat_rend_set.stop_label_font_size);
        tmp_render_settings.add_stop_label_offset(cat_rend_set.stop_label_offset[0]);
        tmp_render_settings.add_stop_label_offset(cat_rend_set.stop_label_offset[1]);
        *tmp_render_settings.mutable_underlayer_color() = SerializeColor(cat_rend_set.underlayer_color);
        tmp_render_settings.set_underlayer_width(cat_rend_set.underlayer_width);

        for (int i = 0; i < cat_rend_set.color_palette.size(); ++i) {
            tmp_render_settings.add_color_palette();
            *tmp_render_settings.mutable_color_palette(i) = SerializeColor(cat_rend_set.color_palette[i]);
        }

        return tmp_render_settings;
    }

    transport_data_base::Color TransportCatalogueSerialization::SerializeColor(const svg::Color& color) {
        transport_data_base::Color tmp_color;
        if (std::holds_alternative<std::monostate>(color)) {
            return {};
        }
        else if (std::holds_alternative<std::string>(color)) {
            tmp_color.set_name(std::get<std::string>(color));
        }
        else if (std::holds_alternative<svg::Rgb>(color)) {
            svg::Rgb tmp_rgb_color = std::get<svg::Rgb>(color);
            tmp_color.mutable_rgb()->set_red(tmp_rgb_color.red);
            tmp_color.mutable_rgb()->set_green(tmp_rgb_color.green);
            tmp_color.mutable_rgb()->set_blue(tmp_rgb_color.blue);
        }
        else {
            svg::Rgba tmp_rgba_color = std::get<svg::Rgba>(color);
            tmp_color.mutable_rgba()->set_red(tmp_rgba_color.red);
            tmp_color.mutable_rgba()->set_green(tmp_rgba_color.green);
            tmp_color.mutable_rgba()->set_blue(tmp_rgba_color.blue);
            tmp_color.mutable_rgba()->set_opacity(tmp_rgba_color.opacity);
        }
        return tmp_color;
    }

    void TransportCatalogueSerialization::Deserialize() {
        std::filesystem::path path = settings_.file_name;
        std::ifstream in_file(path, std::ios::binary);

        base_.ParseFromIstream(&in_file);

        DeserializeCatalogue(base_.transport_catalogue());
        DeserializeMapRenderer(base_.map_renderer());
    }

    void TransportCatalogueSerialization::DeserializeCatalogue(const transport_data_base::Catalogue& base) {

        for (int i = 0; i < base.stops_size(); ++i) {
            catalogue_.AddStop({base.stops(i).name(),
                                base.stops(i).coord().lat(),
                                base.stops(i).coord().lng()});
        }

        for (int i = 0; i < base.buses_size(); ++i) {
            std::vector<std::string_view> stops;
            for (int j = 0; j < base.buses(i).stop_names_size(); ++j) {
                stops.push_back(base.buses(i).stop_names(j));
            }
            catalogue_.AddBus(base.buses(i).number(), stops, base.buses(i).circle());
        }

        for (int i = 0; i < base.distances_size(); ++i) {
            catalogue_.SetDistances(base.distances(i).from(),
                                    base.distances(i).to(),
                                    base.distances(i).distance());
        }
    }

    void TransportCatalogueSerialization::DeserializeMapRenderer(const transport_data_base::MapRenderer& base) {
        renderer_.SetSettings(DeserializeMapRenderSettings(base.settings()));
    }

    map_renderer::RenderSettings TransportCatalogueSerialization::DeserializeMapRenderSettings(const transport_data_base::RenderSettings& base_render_settings) {
        map_renderer::RenderSettings tmp_settings;
        tmp_settings.width = base_render_settings.width();
        tmp_settings.height = base_render_settings.height();
        tmp_settings.padding = base_render_settings.padding();
        tmp_settings.line_width = base_render_settings.line_width();
        tmp_settings.stop_radius = base_render_settings.stop_radius();
        tmp_settings.bus_label_font_size = base_render_settings.bus_label_font_size();
        tmp_settings.bus_label_offset[0] = base_render_settings.bus_label_offset(0);
        tmp_settings.bus_label_offset[1] = base_render_settings.bus_label_offset(1);
        tmp_settings.stop_label_font_size = base_render_settings.stop_label_font_size();
        tmp_settings.stop_label_offset[0] = base_render_settings.stop_label_offset(0);
        tmp_settings.stop_label_offset[1] = base_render_settings.stop_label_offset(1);
        tmp_settings.underlayer_color = DeserializeColor(base_render_settings.underlayer_color());
        tmp_settings.underlayer_width = base_render_settings.underlayer_width();
        tmp_settings.color_palette.reserve(base_render_settings.color_palette_size());
        for (int i = 0; i < base_render_settings.color_palette_size(); ++i) {
            tmp_settings.color_palette.emplace_back(std::move(DeserializeColor(base_render_settings.color_palette(i))));
        }
        return tmp_settings;
    }

    svg::Color TransportCatalogueSerialization::DeserializeColor(const transport_data_base::Color& base_color) {
        svg::Color empty_color{};
        switch (base_color.data_case()) {
            case transport_data_base::Color::DataCase::DATA_NOT_SET:
                return  empty_color;
                break;
            case transport_data_base::Color::DataCase::kName:
                return base_color.name();
                break;
            case transport_data_base::Color::DataCase::kRgb:
                return svg::Rgb(base_color.rgb().red(),
                                base_color.rgb().green(),
                                base_color.rgb().blue());
                break;
            case transport_data_base::Color::DataCase::kRgba:
                return svg::Rgba(base_color.rgba().red(),
                                 base_color.rgba().green(),
                                 base_color.rgba().blue(),
                                 base_color.rgba().opacity());
                break;
        }
        return  empty_color;
    }
}