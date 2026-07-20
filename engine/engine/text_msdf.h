#ifndef GAMEENGINE_TEXT_MSDF_H
#define GAMEENGINE_TEXT_MSDF_H

#include "vector"

#include "json.h"
#include "dependencies/json.h"

namespace TEXT_MSDF {
static std::string EXAMPLE_FONT_ATLAS_JSON = R"({
  "atlas": {
    "type": "sdf",
    "distanceRange": 23.65625,
    "distanceRangeMiddle": 0,
    "size": 47.3125,
    "width": 511,
    "height": 511,
    "yOrigin": "bottom"
  },
  "metrics": {
    "emSize": 1,
    "lineHeight": 1.14990234375,
    "ascender": 0.9052734375,
    "descender": -0.2119140625,
    "underlineY": -0.142578125,
    "underlineThickness": 0.0732421875
  },
  "glyphs": [
    {
      "unicode": 32,
      "advance": 0.27783203125
    },
    {
      "unicode": 33,
      "advance": 0.27783203125,
      "planeBounds": {
        "left": -0.16609206004375823,
        "bottom": -0.26420079260237783,
        "right": 0.44685377879375832,
        "top": 0.9828269484808454
      },
      "atlasBounds": {
        "left": 99.5,
        "bottom": 382.5,
        "right": 128.5,
        "top": 441.5
      }
    }
  ]
})";

struct Bounds {
    double left;
    double bottom;
    double right;
    double top;
};

struct Glyph {
    int unicode;
    double advance;
    Bounds planeBounds;
    Bounds atlasBounds;
};

struct Metrics {
    double emSize;
    double lineHeight;
    double ascender;
    double descender;
    double underlineY;
    double underlineThickness;
};

struct Atlas {
    const char *type;
    double distanceRange;
    double distanceRangeMiddle;
    double size;
    int width;
    int height;
    const char *yOrigin;
};

struct FontAtlas {
    Atlas atlas;
    Metrics metrics;
    std::vector<Glyph> glyphs;
};

static Metrics fromJsonMetrics(json_object_s *obj) {
    Metrics metrics{};
    json_object_element_s *node = obj->start;
    for (int i = 0; i < obj->length; i++) {
#ifdef JSON_DEBUG
        json_debug_print(node);
#endif
        if (strcmp(node->name->string, "emSize") == 0) {
            metrics.emSize = json_to_int(node->value);
        } else if (strcmp(node->name->string, "lineHeight") == 0) {
            metrics.lineHeight = json_to_float(node->value);
        } else if (strcmp(node->name->string, "ascender") == 0) {
            metrics.ascender = json_to_float(node->value);
        } else if (strcmp(node->name->string, "descender") == 0) {
            metrics.descender = json_to_float(node->value);
        } else if (strcmp(node->name->string, "underlineY") == 0) {
            metrics.underlineY = json_to_float(node->value);
        } else if (strcmp(node->name->string, "underlineThickness") == 0) {
            metrics.underlineThickness = json_to_float(node->value);
        } else {
            assert(false);
        }
        node = node->next;
    }
    return metrics;
}

static Atlas fromJsonAtlas(json_object_s *obj) {
    Atlas atlas{};
    json_object_element_s *node = obj->start;
    for (int i = 0; i < obj->length; i++) {
#ifdef JSON_DEBUG
        json_debug_print(node);
#endif
        if (strcmp(node->name->string, "type") == 0) {
            atlas.type = json_to_string(node->value);
        } else if (strcmp(node->name->string, "yOrigin") == 0) {
            atlas.yOrigin = json_to_string(node->value);
        } else if (strcmp(node->name->string, "size") == 0) {
            atlas.size = json_to_float(node->value);
        } else if (strcmp(node->name->string, "distanceRange") == 0) {
            atlas.distanceRange = json_to_float(node->value);
        } else if (strcmp(node->name->string, "distanceRangeMiddle") == 0) {
            atlas.distanceRangeMiddle = json_to_float(node->value);
        } else if (strcmp(node->name->string, "width") == 0) {
            atlas.width = json_to_int(node->value);
        } else if (strcmp(node->name->string, "height") == 0) {
            atlas.height = json_to_int(node->value);
        }
        node = node->next;
    }
    return atlas;
}

static Bounds fromJsonBounds(json_object_s *obj) {
    Bounds bounds{};
    json_object_element_s *node = obj->start;
    for (int i = 0; i < obj->length; i++) {
#ifdef JSON_DEBUG
        json_debug_print(node);
#endif
        if (strcmp(node->name->string, "top") == 0) {
            bounds.top = json_to_float(node->value);
        } else if (strcmp(node->name->string, "left") == 0) {
            bounds.left = json_to_float(node->value);
        } else if (strcmp(node->name->string, "right") == 0) {
            bounds.right = json_to_float(node->value);
        } else if (strcmp(node->name->string, "bottom") == 0) {
            bounds.bottom = json_to_float(node->value);
        }
        node = node->next;
    }
    return bounds;
}

static Glyph fromJsonGlyph(json_object_s *obj) {
    Glyph glyph{};
    json_object_element_s *node = obj->start;
    for (int i = 0; i < obj->length; i++) {
#ifdef JSON_DEBUG
        json_debug_print(node);
#endif
        if (strcmp(node->name->string, "advance") == 0) {
            glyph.advance = json_to_float(node->value);
        } else if (strcmp(node->name->string, "unicode") == 0) {
            glyph.unicode = json_to_int(node->value);
        } else if (strcmp(node->name->string, "planeBounds") == 0) {
            json_value_s *val = node->value;
            assert(val->type == json_type_object);
            json_object_s *value = json_value_as_object(val);
            glyph.planeBounds = fromJsonBounds(value);
        } else if (strcmp(node->name->string, "atlasBounds") == 0) {
            json_value_s *val = node->value;
            assert(val->type == json_type_object);
            json_object_s *value = json_value_as_object(val);
            glyph.atlasBounds = fromJsonBounds(value);
        }
        node = node->next;
    }
    return glyph;
}

static std::vector<Glyph> fromJsonGlyphs(json_array_s *obj) {
    std::vector<Glyph> glyphs{};
    json_array_element_s *node = obj->start;
    for (int i = 0; i < obj->length; i++) {
        json_value_s *val = node->value;
        json_object_s *el = json_value_as_object(val);
        Glyph glyph = fromJsonGlyph(el);
        glyphs.push_back(glyph);
        node = node->next;
    }
    return glyphs;
}

static FontAtlas fromJsonFontAtlas(const char *_json) {
    FontAtlas atlas{};
    auto res = json_parse(_json, strlen(_json));
    auto *object = (struct json_object_s *) res->payload;
    json_object_element_s *node = object->start;
    for (int i = 0; i < object->length; i++) {
#ifdef JSON_DEBUG
        json_debug_print(node);
#endif
        if (strcmp(node->name->string, "atlas") == 0) {
            json_value_s *val = node->value;
            assert(val->type == json_type_object);
            json_object_s *value = json_value_as_object(val);
            atlas.atlas = fromJsonAtlas(value);
        } else if (strcmp(node->name->string, "metrics") == 0) {
            json_value_s *val = node->value;
            assert(val->type == json_type_object);
            json_object_s *value = json_value_as_object(val);
            atlas.metrics = fromJsonMetrics(value);
        } else if (strcmp(node->name->string, "glyphs") == 0) {
            json_value_s *val = node->value;
            assert(val->type == json_type_array);
            json_array_s *value = json_value_as_array(val);
            atlas.glyphs = fromJsonGlyphs(value);
        }
        node = node->next;
    }
    free(res);
    return atlas;
}
}

#endif //GAMEENGINE_TEXT_MSDF_H