//
// Created by cpasjuste on 21/11/16.
//

#include "GL/gl.h"
#include "sfml_renderer.h"
#include "sfml_font.h"
#include "sfml_texture.h"

//////////
// INIT //
//////////
SFMLRenderer::SFMLRenderer(int w, int h, const std::string &shaderPath) : Renderer() {

    sf::ContextSettings settings(16, 0, 0);
    mode = sf::VideoMode::getDesktopMode();
    sf::Uint32 style = sf::Style::Fullscreen;

    // windowed
    if (w != 0 && h != 0) {
        mode.width = (unsigned int) w;
        mode.height = (unsigned int) h;
        style = sf::Style::Default;
    }

    window.create(mode, "SFMLRenderer", style, settings);
    window.setVerticalSyncEnabled(true);

    printf("SFMLRenderer: %i x %i @ %i bpp\n", mode.width, mode.height, mode.bitsPerPixel);
    const unsigned char *glversion = glGetString(GL_VERSION);
    printf("SFMLRenderer: glversion: %s\n", glversion);
    const unsigned char *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("SFMLRenderer: glslversion: %s\n", glslversion);

    this->shaders = (Shaders *) new SFMLShaders(shaderPath);
}
//////////
// INIT //
//////////

//////////
// FONT //
//////////
Font *SFMLRenderer::LoadFont(const char *path, int size) {
    Font *font = (Font *) new SFMLFont(path, size);
    return font;
}

void SFMLRenderer::DrawFont(Font *font, int x, int y, const char *fmt, ...) {
    if (font == NULL) {
        return;
    }

    char msg[MAX_PATH];
    memset(msg, 0, MAX_PATH);
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, MAX_PATH, fmt, args);
    va_end(args);

    SFMLFont *fnt = (SFMLFont *) font;
    sf::Text text;
    text.setFont(fnt->font);
    text.setString(msg);
    text.setCharacterSize((unsigned int) font->size);

    text.setFillColor(sf::Color(
            font->color.r, font->color.g,
            font->color.b, font->color.a));
    text.setOutlineColor(sf::Color::Black);
    text.setOutlineThickness(2);

    text.setOrigin(text.getLocalBounds().left, text.getLocalBounds().top);
    text.scale(font->scaling, font->scaling);
    text.setPosition(x, y);

    window.draw(text);
}

//////////
// FONT //
//////////

/////////////
// TEXTURE //
/////////////
Texture *SFMLRenderer::CreateTexture(int w, int h) {
    SFMLTexture *texture = new SFMLTexture(w, h);
    if (!texture->pixels) {
        return NULL;
    }
    return (Texture *) texture;
}

Texture *SFMLRenderer::LoadTexture(const char *file) {
    SFMLTexture *texture = new SFMLTexture(file);
    if (!texture->pixels) {
        return NULL;
    }
    return (Texture *) texture;
}

void SFMLRenderer::DrawTexture(Texture *texture, int x, int y, int w, int h, float rotation) {

    sf::RenderStates states;

    sf::Sprite sprite = ((SFMLTexture *) texture)->sprite;

    // set sprite position
    sprite.setPosition(x, y);

    // set sprite scaling
    float scaleX = (float) w / (float) texture->width;
    float scaleY = (float) h / (float) texture->height;
    sprite.setScale(scaleX, scaleY);

    // set sprite rotation
    sf::Transform transform;
    transform.rotate(rotation, {(float) (x + w / 2), (float) (y + h / 2)});
    states.transform = transform;

    // set sprite shader
    sf::Shader *shader = (sf::Shader *) shaders->Get()->data;
    if (shader) {
        shader->setUniform("Texture", ((SFMLTexture *) texture)->texture);
        shader->setUniform("MVPMatrix", sf::Glsl::Mat4(window.getView().getTransform().getMatrix()));
        shader->setUniform("TextureSize", sf::Glsl::Vec2(texture->width, texture->height));
        shader->setUniform("InputSize", sf::Glsl::Vec2(w, h));
        shader->setUniform("OutputSize", sf::Glsl::Vec2(w, h));
        states.shader = shader;
    }

    // draw sprite
    window.draw(sprite, states);
}

int SFMLRenderer::LockTexture(Texture *texture, const Rect &rect, void **pixels, int *pitch) {
    *pixels = ((SFMLTexture *) texture)->pixels;
    *pitch = texture->width * 2;
    return 0;
}

void SFMLRenderer::UnlockTexture(Texture *texture) {
    GLint textureBinding;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding);
    sf::Texture::bind(&((SFMLTexture *) texture)->texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->width, texture->height,
                    GL_RGB, GL_UNSIGNED_SHORT_5_6_5, ((SFMLTexture *) texture)->pixels);
    glBindTexture(GL_TEXTURE_2D, (GLuint) textureBinding);
}

/////////////
// TEXTURE //
/////////////

const Rect SFMLRenderer::GetWindowSize() {
    Rect rect{0, 0, (int)window.getSize().x, (int)window.getSize().y};
    return rect;
}

void SFMLRenderer::SetShader(int index) {
    if (index == shaders->current || index >= shaders->Count()) {
        return;
    }
    shaders->current = index;
}

void SFMLRenderer::DrawLine(int x1, int y1, int x2, int y2, const Color &c) {

    sf::Color col(c.r, c.g, c.b, c.a);

    sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x1, y1), col),
            sf::Vertex(sf::Vector2f(x2, y2), col)
    };

    window.draw(line, 2, sf::Lines);
}

void SFMLRenderer::DrawRect(const Rect &rect, const Color &c, bool fill) {

    sf::Color col(c.r, c.g, c.b, c.a);
    sf::RectangleShape rectangle(sf::Vector2f(rect.w-2, rect.h-2));
    rectangle.setOutlineColor(col);
    rectangle.setOutlineThickness(1);
    rectangle.setPosition(rect.x+1, rect.y+1);
    if (fill) {
        rectangle.setFillColor(col);
    } else {
        rectangle.setFillColor(sf::Color(0, 0, 0, 0));
    }
    window.draw(rectangle);
}

void SFMLRenderer::Clip(const Rect &rect) {
}

void SFMLRenderer::Clear() {
    window.clear(
            sf::Color(color.r, color.g, color.b, color.a));
}

void SFMLRenderer::Flip() {
    window.display();
}

void SFMLRenderer::Delay(unsigned int ms) {
    sf::sleep(sf::milliseconds(ms));
}

SFMLRenderer::~SFMLRenderer() {
    delete (shaders);
    window.close();
}
