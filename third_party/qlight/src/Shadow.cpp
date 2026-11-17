#include "QLight/Shadow.h"

#include <QHash>
#include <QImage>
#include <QPainter>
#include <QPixmap>

#include <cmath>
#include <cstring>
#include <vector>

namespace QLight {
namespace Shadow {

namespace {

struct Key {
    ShadowSpec spec;
    int dprPercent;
    bool operator==(const Key &o) const { return spec == o.spec && dprPercent == o.dprPercent; }
};

size_t qHash(const Key &k, size_t seed = 0)
{
    return qHashMulti(seed, k.spec.radius, k.spec.blur, k.spec.offsetY, k.spec.color.rgba(),
                      k.dprPercent);
}

// One horizontal box-blur pass over an 8-bit alpha buffer (running sum).
void boxBlurH(const uchar *src, uchar *dst, int w, int h, int r)
{
    const int win = 2 * r + 1;
    for (int y = 0; y < h; ++y) {
        const uchar *s = src + y * w;
        uchar *d = dst + y * w;
        int sum = 0;
        for (int x = 0; x <= r && x < w; ++x) // out-of-range samples count as 0
            sum += s[x];
        for (int x = 0; x < w; ++x) {
            d[x] = uchar(sum / win);
            const int add = x + r + 1, sub = x - r;
            sum += (add < w ? s[add] : 0) - (sub >= 0 ? s[sub] : 0);
        }
    }
}

void transpose(const uchar *src, uchar *dst, int w, int h)
{
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            dst[x * h + y] = src[y * w + x];
}

// Approximate gaussian: three box blurs per axis.
void blurAlpha(std::vector<uchar> &buf, int w, int h, int r)
{
    if (r < 1)
        return;
    std::vector<uchar> tmp(buf.size());
    for (int pass = 0; pass < 3; ++pass) {
        boxBlurH(buf.data(), tmp.data(), w, h, r);
        buf.swap(tmp);
    }
    transpose(buf.data(), tmp.data(), w, h);
    buf.swap(tmp);
    for (int pass = 0; pass < 3; ++pass) {
        boxBlurH(buf.data(), tmp.data(), h, w, r);
        buf.swap(tmp);
    }
    transpose(buf.data(), tmp.data(), h, w);
    buf.swap(tmp);
}

struct Patch {
    QPixmap pixmap;
    qreal corner = 0; // logical size of each corner cell (extent + radius)
    qreal core = 1;   // logical size of the stretchable middle
};

Patch render(const ShadowSpec &spec, qreal dpr)
{
    Patch patch;
    const qreal e = extent(spec);
    patch.corner = e + spec.radius;
    const qreal logical = 2 * patch.corner + patch.core;
    const int px = qMax(1, int(std::ceil(logical * dpr)));

    // Silhouette of the casting rect, centered, at device resolution.
    QImage sil(px, px, QImage::Format_Alpha8);
    sil.fill(0);
    {
        QPainter p(&sil);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::black);
        const qreal in = e * dpr;
        p.drawRoundedRect(QRectF(in, in, px - 2 * in, px - 2 * in), spec.radius * dpr,
                          spec.radius * dpr);
    }

    std::vector<uchar> buf(size_t(px) * px);
    for (int y = 0; y < px; ++y)
        std::memcpy(buf.data() + size_t(y) * px, sil.constScanLine(y), size_t(px));
    // box-blur radius from desired extent: 3 passes of radius r spread ~1.5*r
    blurAlpha(buf, px, px, qMax(1, int(std::round(spec.blur * dpr / 2.0))));

    QImage out(px, px, QImage::Format_ARGB32_Premultiplied);
    const QColor c = spec.color;
    for (int y = 0; y < px; ++y) {
        auto *line = reinterpret_cast<QRgb *>(out.scanLine(y));
        for (int x = 0; x < px; ++x) {
            const int a = buf[size_t(y) * px + x] * c.alpha() / 255;
            line[x] = qRgba(c.red() * a / 255, c.green() * a / 255, c.blue() * a / 255, a);
        }
    }
    patch.pixmap = QPixmap::fromImage(out);
    patch.pixmap.setDevicePixelRatio(dpr);
    return patch;
}

const Patch &patchFor(const ShadowSpec &spec, qreal dpr)
{
    static QHash<Key, Patch> cache;
    const Key key{spec, int(std::round(dpr * 100))};
    auto it = cache.find(key);
    if (it == cache.end())
        it = cache.insert(key, render(spec, dpr));
    return it.value();
}

} // namespace

qreal extent(const ShadowSpec &spec)
{
    return std::ceil(spec.blur * 1.5);
}

void draw(QPainter *p, const QRectF &rect, const ShadowSpec &spec, qreal dpr)
{
    if (spec.color.alpha() == 0 || rect.isEmpty())
        return;

    const Patch &patch = patchFor(spec, dpr);
    const qreal e = extent(spec);
    const QRectF t = rect.translated(0, spec.offsetY).adjusted(-e, -e, e, e); // shadow bounds
    const qreal c = patch.corner;

    p->save();
    p->setRenderHint(QPainter::SmoothPixmapTransform);

    // Too small for a 9-patch: stretch the whole thing.
    if (t.width() < 2 * c || t.height() < 2 * c) {
        p->drawPixmap(t, patch.pixmap, QRectF(QPointF(0, 0), QSizeF(patch.pixmap.size())));
        p->restore();
        return;
    }

    const int pw = patch.pixmap.width(), ph = patch.pixmap.height();
    const int cp = int(std::round(c * dpr));            // corner in device px
    const int mid = pw - 2 * cp;                        // middle slice in device px
    const int midH = ph - 2 * cp;
    const qreal xs[4] = {t.left(), t.left() + c, t.right() - c, t.right()};
    const qreal ys[4] = {t.top(), t.top() + c, t.bottom() - c, t.bottom()};
    const int sx[4] = {0, cp, cp + mid, pw};
    const int sy[4] = {0, cp, cp + midH, ph};

    for (int j = 0; j < 3; ++j) {
        for (int i = 0; i < 3; ++i) {
            const QRectF dst(QPointF(xs[i], ys[j]), QPointF(xs[i + 1], ys[j + 1]));
            const QRectF src(QPointF(sx[i], sy[j]), QPointF(sx[i + 1], sy[j + 1]));
            p->drawPixmap(dst, patch.pixmap, src);
        }
    }
    p->restore();
}

} // namespace Shadow
} // namespace QLight
