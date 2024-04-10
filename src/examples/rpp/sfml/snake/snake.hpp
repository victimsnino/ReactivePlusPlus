#pragma once

#include <rpp/fwd.hpp>

#include "utils.hpp"

rpp::dynamic_observable<sf::RectangleShape> get_shapes_to_draw(const rpp::dynamic_observable<CustomEvent>& events);
