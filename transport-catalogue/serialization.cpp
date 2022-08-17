#include "serialization.h"

namespace transport_data_base {

    void TransportCatalogueSerialization::SetSettings(const SerializationSettings& settings) {
        settings_ = settings;
    }

    void TransportCatalogueSerialization::Serialize(){
        *base_.mutable_transport_catalogue() = std::move(SerializeCatalogue());
        *base_.mutable_map_renderer() = std::move(SerializeMapRenderer());
        *base_.mutable_transport_router() = std::move(SerializeTransportRouter());

        std::filesystem::path path = settings_.file_name;
        std::ofstream out_file(path, std::ios::binary);
        base_.SerializeToOstream(&out_file);
    }

    transport_data_base::Catalogue
    TransportCatalogueSerialization::SerializeCatalogue() {
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

    transport_data_base::Stop
    TransportCatalogueSerialization::SerializeStop(const domain::Stop* stop_data) {
        transport_data_base::Stop tmp_stop;
        tmp_stop.set_name(std::string(stop_data->name_));
        tmp_stop.mutable_coord()->set_lat(stop_data->coord_.lat);
        tmp_stop.mutable_coord()->set_lng(stop_data->coord_.lng);
        return tmp_stop;
    }

    transport_data_base::Bus
    TransportCatalogueSerialization::SerializeBus(const domain::Bus* bus_data) {
        transport_data_base::Bus tmp_bus;
        tmp_bus.set_number(std::string(bus_data->number_));
        for (const auto stop : bus_data->stop_names_) {
            tmp_bus.add_stop_names(stop->name_);
        }
        tmp_bus.set_circle(bus_data->circle);
        return tmp_bus;
    }

    transport_data_base::Distance
    TransportCatalogueSerialization::SerializeDistance(const std::pair<domain::Stop*, domain::Stop*> stops,
                                                                                     size_t length) {
        transport_data_base::Distance tmp_dist;
        tmp_dist.set_from(stops.first->name_);
        tmp_dist.set_to(stops.second->name_);
        tmp_dist.set_distance(static_cast<uint32_t>(length));
        return tmp_dist;
    }

    transport_data_base::MapRenderer
    TransportCatalogueSerialization::SerializeMapRenderer() {
        transport_data_base::MapRenderer tmp_map;
        *tmp_map.mutable_settings() = std::move(SerializeRenderSettings());
        return tmp_map;
    }

    transport_data_base::RenderSettings
    TransportCatalogueSerialization::SerializeRenderSettings() {
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

    transport_data_base::Color
    TransportCatalogueSerialization::SerializeColor(const svg::Color& color) {
        transport_data_base::Color tmp_color;
        if (std::holds_alternative<std::monostate>(color)) {
            return {};
        } else if (std::holds_alternative<std::string>(color)) {
            tmp_color.set_name(std::get<std::string>(color));
        } else if (std::holds_alternative<svg::Rgb>(color)) {
            svg::Rgb tmp_rgb_color = std::get<svg::Rgb>(color);
            tmp_color.mutable_rgb()->set_red(tmp_rgb_color.red);
            tmp_color.mutable_rgb()->set_green(tmp_rgb_color.green);
            tmp_color.mutable_rgb()->set_blue(tmp_rgb_color.blue);
        } else {
            svg::Rgba tmp_rgba_color = std::get<svg::Rgba>(color);
            tmp_color.mutable_rgba()->set_red(tmp_rgba_color.red);
            tmp_color.mutable_rgba()->set_green(tmp_rgba_color.green);
            tmp_color.mutable_rgba()->set_blue(tmp_rgba_color.blue);
            tmp_color.mutable_rgba()->set_opacity(tmp_rgba_color.opacity);
        }
        return tmp_color;
    }

    transport_data_base::TransportRouter
    TransportCatalogueSerialization::SerializeTransportRouter() {
        transport_data_base::TransportRouter tmp_router;

        *tmp_router.mutable_settings() = std::move(SerializeRouterSettingsData());
        *tmp_router.mutable_transport_router() = std::move(SerializeTransportRouterClass());
        *tmp_router.mutable_router() = std::move(SerializeRouter());
        *tmp_router.mutable_graph() = std::move(SerializeGraph());

        return tmp_router;
    }

    transport_data_base::RouterSettings
    TransportCatalogueSerialization::SerializeRouterSettingsData() {
        transport_data_base::RouterSettings tmp_router_settings;
        transport_router::RouterSettings cat_router_set = router_.GetSettings();
        tmp_router_settings.set_bus_velocity_kmh(cat_router_set.bus_velocity);
        tmp_router_settings.set_bus_wait_time(cat_router_set.bus_wait_time);
        return tmp_router_settings;
    }

    transport_data_base::TransportRouterData
    TransportCatalogueSerialization::SerializeTransportRouterClass() {
        transport_data_base::TransportRouterData tmp_tr_router_cd;
        int i = 0;
        for (const auto& [stop, id] : *router_.GetStopToId()) {
            tmp_tr_router_cd.add_stop_to_id_();
            tmp_tr_router_cd.mutable_stop_to_id_(i)->set_stop(std::string(stop));
            tmp_tr_router_cd.mutable_stop_to_id_(i)->set_id(id);
            ++i;
        }
        int j = 0;
        for (const auto& [id, stop] : *router_.GetIdToStop()) {
            tmp_tr_router_cd.add_id_to_stop_();
            tmp_tr_router_cd.mutable_id_to_stop_(j)->set_id(id);
            tmp_tr_router_cd.mutable_id_to_stop_(j)->set_stop(std::string(stop));
            ++j;
        }
        int k = 0;
        for (const auto& [id, bus] : *router_.GetEdgeIdToBus()) {
            tmp_tr_router_cd.add_edge_id_to_bus_();
            tmp_tr_router_cd.mutable_edge_id_to_bus_(k)->set_edge_id(id);
            tmp_tr_router_cd.mutable_edge_id_to_bus_(k)->set_bus(std::string(bus));
            ++k;
        }
        int h = 0;
        for (const auto& [id, count] : *router_.GetEdgeIdToSpanCount()) {
            tmp_tr_router_cd.add_edge_id_to_span_count_();
            tmp_tr_router_cd.mutable_edge_id_to_span_count_(h)->set_edge_id(id);
            tmp_tr_router_cd.mutable_edge_id_to_span_count_(h)->set_span_count(count);
            ++h;
        }
        return tmp_tr_router_cd;
    }

    transport_data_base::Router
    TransportCatalogueSerialization::SerializeRouter() {
        transport_data_base::Router tmp_router;
        int i = 0;
        for (const auto& data_vector : router_.GetRouterData()) {
            int j = 0;
            tmp_router.add_routes_internal_data();
            for (const auto& data : data_vector) {
                tmp_router.mutable_routes_internal_data(i)->add_route_internal_data_elem();
                if (data) {
                    transport_data_base::RouteInternalData&
                    elem_data = *tmp_router.mutable_routes_internal_data(i)->
                            mutable_route_internal_data_elem(j)->
                            mutable_data(); // tmp element
                    elem_data.set_weight(data.value().weight);
                    if (data.value().prev_edge) {
                        elem_data.set_edgeid(data.value().prev_edge.value());
                    }
                }
                ++j;
            }
            ++i;
        }
        return tmp_router;
    }

    transport_data_base::Graph
    TransportCatalogueSerialization::SerializeGraph() {
        transport_data_base::Graph tmp_graph;
        for (int i = 0; i < router_.GetGraph().GetEdgeCount(); ++i) {
            graph::Edge tmp_cat_edge = router_.GetGraph().GetEdge(i);
            tmp_graph.add_edges();
            transport_data_base::Edge& tmp_base_edge = *tmp_graph.mutable_edges(i);
            tmp_base_edge.set_from(tmp_cat_edge.from);
            tmp_base_edge.set_to(tmp_cat_edge.to);
            tmp_base_edge.set_weight(tmp_cat_edge.weight);
        }
        for (size_t i = 0; i < router_.GetGraph().GetVertexCount(); ++i) {
            tmp_graph.add_incidence_lists();
            transport_data_base::IncidenceList& tmp_base_inc_list = *tmp_graph.mutable_incidence_lists(i);

            int j = 0;
            for (const auto inc_edge : router_.GetGraph().GetIncidentEdges(i)) {
                tmp_base_inc_list.add_edges(inc_edge);
                ++j;
            }
        }
        return tmp_graph;
    }

    void TransportCatalogueSerialization::Deserialize() {
        std::filesystem::path path = settings_.file_name;
        std::ifstream in_file(path, std::ios::binary);

        base_.ParseFromIstream(&in_file);

        DeserializeCatalogue(base_.transport_catalogue());
        DeserializeMapRenderer(base_.map_renderer());

        router_.GenerateEmptyRouter();

        DeserializeTransportRouterData(base_.transport_router());
    }
    // Transport Catalogue
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
            catalogue_.AddBus(base.buses(i).number(),
                              stops,
                              base.buses(i).circle());
        }

        for (int i = 0; i < base.distances_size(); ++i) {
            catalogue_.SetDistances(base.distances(i).from(),
                                    base.distances(i).to(),
                                    base.distances(i).distance());
        }
    }
    // Map renderer
    void TransportCatalogueSerialization::DeserializeMapRenderer(const transport_data_base::MapRenderer& base) {
        renderer_.SetSettings(DeserializeMapRenderSettings(base.settings()));
    }

    map_renderer::RenderSettings
    TransportCatalogueSerialization::DeserializeMapRenderSettings(const transport_data_base::RenderSettings& base_render_settings) {
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

    svg::Color
    TransportCatalogueSerialization::DeserializeColor(const transport_data_base::Color& base_color) {
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
    // Transport router
    void TransportCatalogueSerialization::DeserializeTransportRouterData(const transport_data_base::TransportRouter& base) {
        router_.SetSettings(DeserializeTransportRouterSettings(base.settings()));
        DeserializeTransportRouterClass(base.transport_router());
        DeserializeRouter(base_.transport_router().router());
        DeserializeGraph(base_.transport_router().graph());
    }

    transport_router::RouterSettings
    TransportCatalogueSerialization::DeserializeTransportRouterSettings(const transport_data_base::RouterSettings& base) {
        transport_router::RouterSettings tmp_settings;
        tmp_settings.bus_velocity = base.bus_velocity_kmh();
        tmp_settings.bus_wait_time = base.bus_wait_time();
        return tmp_settings;
    }

    void TransportCatalogueSerialization::DeserializeTransportRouterClass(const transport_data_base::TransportRouterData& base) {
        router_.SetStopToId() = std::move(DeserializeStopToId(base));
        router_.SetIdToStop() = std::move(DeserializeIdToStop(base));
        router_.SetEdgeIdToBus() = std::move(DeserializeEdgeIdToBus(base));
        router_.SetEdgeIdToSpanCount() = std::move(DeserializeEdgeIdToSpanCount(base));
    }

    std::unordered_map<std::string_view, size_t>
    TransportCatalogueSerialization::DeserializeStopToId(const transport_data_base::TransportRouterData& base) {
        std::unordered_map<std::string_view, size_t> tmp_stop_to_id;
        for (int i = 0; i < base.stop_to_id__size(); ++i) {
            tmp_stop_to_id[base.stop_to_id_(i).stop()] = base.stop_to_id_(i).id();
        }
        return tmp_stop_to_id;
    }

    std::unordered_map<size_t, std::string_view>
    TransportCatalogueSerialization::DeserializeIdToStop(const transport_data_base::TransportRouterData& base) {
        std::unordered_map<size_t, std::string_view> tmp_id_to_stop;
        for (int i = 0; i < base.id_to_stop__size(); ++i) {
            tmp_id_to_stop[base.id_to_stop_(i).id()] = base.id_to_stop_(i).stop();
        }
        return tmp_id_to_stop;
    }

    std::unordered_map<size_t, std::string_view>
    TransportCatalogueSerialization::DeserializeEdgeIdToBus(const transport_data_base::TransportRouterData& base) {
        std::unordered_map<size_t, std::string_view> tmp_edge_bus;
        for (int i = 0; i < base.edge_id_to_bus__size(); ++i) {
            tmp_edge_bus[base.edge_id_to_bus_(i).edge_id()] = base.edge_id_to_bus_(i).bus();
        }
        return tmp_edge_bus;
    }
    std::unordered_map<size_t, size_t>
    TransportCatalogueSerialization::DeserializeEdgeIdToSpanCount(const transport_data_base::TransportRouterData& base) {
        std::unordered_map<size_t, size_t> tmp_edge_span;
        for (int i = 0; i < base.edge_id_to_span_count__size(); ++i) {
            tmp_edge_span[base.edge_id_to_span_count_(i).edge_id()] = base.edge_id_to_span_count_(i).span_count();
        }
        return tmp_edge_span;
    }
    // Router
    void TransportCatalogueSerialization::DeserializeRouter(const transport_data_base::Router& base) {
        std::vector<std::vector<std::optional<graph::Router<double>::RouteInternalData>>>&
        routes_internal_data = router_.ModifyRouter().get()->ModifyRoutesInternalData();

        routes_internal_data.resize(base.routes_internal_data_size());
        for (int i = 0; i < base.routes_internal_data_size(); ++i) {
            routes_internal_data[i].reserve(base.routes_internal_data(i).route_internal_data_elem_size());
            for (int j = 0; j < base.routes_internal_data(i).route_internal_data_elem_size(); ++j) {
                transport_data_base::RouteInternalDataVectorElem
                base_elem = base.routes_internal_data(i).route_internal_data_elem(j);

                routes_internal_data[i].emplace_back(std::move(DeserializeRouteInternal(base_elem)));
            }
        }
    }

    std::optional<graph::Router<double>::RouteInternalData>
    TransportCatalogueSerialization::DeserializeRouteInternal(transport_data_base::RouteInternalDataVectorElem& base) {
        graph::Router<double>::RouteInternalData res{};
        switch (base.elem_case()) {
            case transport_data_base::RouteInternalDataVectorElem::ElemCase::ELEM_NOT_SET:
                return std::nullopt;
                break;
            case transport_data_base::RouteInternalDataVectorElem::ElemCase::kData:
                res.weight = base.data().weight();
                switch (base.data().prev_edge_case()) {
                    case transport_data_base::RouteInternalData::PrevEdgeCase::kEdgeid:
                        res.prev_edge = std::make_optional(base.data().edgeid());
                        break;
                    case transport_data_base::RouteInternalData::PrevEdgeCase::PREV_EDGE_NOT_SET:
                        res.prev_edge = std::nullopt;
                        break;
                }
        }
        return res;
    }
    // Graph
    void TransportCatalogueSerialization::DeserializeGraph(const transport_data_base::Graph & base) {
        router_.ModifyGraph().ModifyEdges() = std::move(DeserializeGraphEdges(base));
        router_.ModifyGraph().ModifyIncidenceLists() = std::move(DeserializeGraphIncidenceLists(base));
    }

    std::vector<graph::Edge<double>>
    TransportCatalogueSerialization::DeserializeGraphEdges(const transport_data_base::Graph& base) {
        std::vector<graph::Edge<double>> tmp_edges;;
        tmp_edges.reserve(base.edges_size());
        for (int i = 0; i < base.edges_size(); ++i) {
            graph::Edge<double> tmp_edge;
            tmp_edge.from = base.edges(i).from();
            tmp_edge.to = base.edges(i).to();
            tmp_edge.weight = base.edges(i).weight();
            tmp_edges.emplace_back(std::move(tmp_edge));
        }
        return tmp_edges;
    }

    std::vector<graph::IncidenceList>
    TransportCatalogueSerialization::DeserializeGraphIncidenceLists(const transport_data_base::Graph& base) {
        std::vector<graph::IncidenceList> tmp_inc_lists;
        tmp_inc_lists.reserve(base.incidence_lists_size());
        for (int i = 0; i < base.incidence_lists_size(); ++i) {
            graph::IncidenceList tmp_list;
            tmp_list.reserve(base.incidence_lists(i).edges_size());
            for (int j = 0; j < base.incidence_lists(i).edges_size(); ++j) {
                tmp_list.emplace_back(base.incidence_lists(i).edges(j));
            }
            tmp_inc_lists.emplace_back(std::move(tmp_list));
        }
        return tmp_inc_lists;
    }
}