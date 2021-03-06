// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "Skin.hpp"
#include <QJsonArray>
#include <QJsonObject>
#include <boost/assign/list_of.hpp>

namespace score
{
// Taken from http://stackoverflow.com/a/31841462
template <typename L, typename R>
boost::bimap<L, R>
make_bimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list)
{
  return boost::bimap<L, R>(list.begin(), list.end());
}

#define SCORE_INSERT_COLOR(Col) \
  {                              \
    #Col, &Col                   \
  }
Skin::Skin() noexcept
    : SansFont{"Ubuntu"}
    , MonoFont{"APCCourier-Bold", 10, QFont::Black}
    , m_colorMap(make_bimap<QString, QBrush*>(
          {SCORE_INSERT_COLOR(Dark),
           SCORE_INSERT_COLOR(HalfDark),
           SCORE_INSERT_COLOR(Gray),
           SCORE_INSERT_COLOR(HalfLight),
           SCORE_INSERT_COLOR(Light),
           SCORE_INSERT_COLOR(Emphasis1),
           SCORE_INSERT_COLOR(Emphasis2),
           SCORE_INSERT_COLOR(Emphasis3),
           SCORE_INSERT_COLOR(Emphasis4),
           SCORE_INSERT_COLOR(Base1),
           SCORE_INSERT_COLOR(Base2),
           SCORE_INSERT_COLOR(Base3),
           SCORE_INSERT_COLOR(Base4),
           SCORE_INSERT_COLOR(Warn1),
           SCORE_INSERT_COLOR(Warn2),
           SCORE_INSERT_COLOR(Warn3),
           SCORE_INSERT_COLOR(Background1),
           SCORE_INSERT_COLOR(Transparent1),
           SCORE_INSERT_COLOR(Transparent2),
           SCORE_INSERT_COLOR(Transparent3),
           SCORE_INSERT_COLOR(Smooth1),
           SCORE_INSERT_COLOR(Smooth2),
           SCORE_INSERT_COLOR(Smooth3),
           SCORE_INSERT_COLOR(Tender1),
           SCORE_INSERT_COLOR(Tender2),
           SCORE_INSERT_COLOR(Tender3),
           SCORE_INSERT_COLOR(Pulse1),
           SCORE_INSERT_COLOR(Pulse2)
                   }))
{
  this->startTimer(32, Qt::CoarseTimer);
  MonoFont.setStyleHint(QFont::Monospace, QFont::PreferQuality);
  MonoFont.setHintingPreference(QFont::PreferFullHinting);
}

Skin& score::Skin::instance()
{
  static Skin s;
  return s;
}

#define SCORE_CONVERT_COLOR(Col) \
  do                              \
  {                               \
    fromColor(#Col, Col);         \
  } while (0)
void Skin::load(const QJsonObject& obj)
{
  auto fromColor = [&](const QString& key, QBrush& col) {
    auto arr = obj[key].toArray();
    if (arr.size() == 3)
      col = QColor(arr[0].toInt(), arr[1].toInt(), arr[2].toInt());
    else if (arr.size() == 4)
      col = QColor(
          arr[0].toInt(), arr[1].toInt(), arr[2].toInt(), arr[3].toInt());
  };

  SCORE_CONVERT_COLOR(Dark);
  SCORE_CONVERT_COLOR(HalfDark);
  SCORE_CONVERT_COLOR(Gray);
  SCORE_CONVERT_COLOR(HalfLight);
  SCORE_CONVERT_COLOR(Light);

  SCORE_CONVERT_COLOR(Emphasis1);
  SCORE_CONVERT_COLOR(Emphasis2);
  SCORE_CONVERT_COLOR(Emphasis3);
  SCORE_CONVERT_COLOR(Emphasis4);

  SCORE_CONVERT_COLOR(Base1);
  SCORE_CONVERT_COLOR(Base2);
  SCORE_CONVERT_COLOR(Base3);
  SCORE_CONVERT_COLOR(Base4);

  SCORE_CONVERT_COLOR(Warn1);
  SCORE_CONVERT_COLOR(Warn2);
  SCORE_CONVERT_COLOR(Warn3);

  SCORE_CONVERT_COLOR(Background1);

  SCORE_CONVERT_COLOR(Transparent1);
  SCORE_CONVERT_COLOR(Transparent2);
  SCORE_CONVERT_COLOR(Transparent3);

  SCORE_CONVERT_COLOR(Smooth1);
  SCORE_CONVERT_COLOR(Smooth2);
  SCORE_CONVERT_COLOR(Smooth3);

  SCORE_CONVERT_COLOR(Tender1);
  SCORE_CONVERT_COLOR(Tender2);
  SCORE_CONVERT_COLOR(Tender3);

  SCORE_CONVERT_COLOR(Pulse1);
  SCORE_CONVERT_COLOR(Pulse2);

  emit changed();
}

#define SCORE_MAKE_PAIR_COLOR(Col) \
  vec.push_back(qMakePair(Col.color(), QStringLiteral(#Col)));
QVector<QPair<QColor, QString>> Skin::getColors() const
{
  QVector<QPair<QColor, QString>> vec;
  vec.reserve(27);

  SCORE_MAKE_PAIR_COLOR(Dark);
  SCORE_MAKE_PAIR_COLOR(HalfDark);
  SCORE_MAKE_PAIR_COLOR(Gray);
  SCORE_MAKE_PAIR_COLOR(HalfLight);
  SCORE_MAKE_PAIR_COLOR(Light);
  SCORE_MAKE_PAIR_COLOR(Emphasis1);
  SCORE_MAKE_PAIR_COLOR(Emphasis2);
  SCORE_MAKE_PAIR_COLOR(Emphasis3);
  SCORE_MAKE_PAIR_COLOR(Emphasis4);
  SCORE_MAKE_PAIR_COLOR(Base1);
  SCORE_MAKE_PAIR_COLOR(Base2);
  SCORE_MAKE_PAIR_COLOR(Base3);
  SCORE_MAKE_PAIR_COLOR(Base4);
  SCORE_MAKE_PAIR_COLOR(Warn1);
  SCORE_MAKE_PAIR_COLOR(Warn2);
  SCORE_MAKE_PAIR_COLOR(Warn3);
  SCORE_MAKE_PAIR_COLOR(Background1);
  SCORE_MAKE_PAIR_COLOR(Transparent1);
  SCORE_MAKE_PAIR_COLOR(Transparent2);
  SCORE_MAKE_PAIR_COLOR(Transparent3);
  SCORE_MAKE_PAIR_COLOR(Smooth1);
  SCORE_MAKE_PAIR_COLOR(Smooth2);
  SCORE_MAKE_PAIR_COLOR(Smooth3);
  SCORE_MAKE_PAIR_COLOR(Tender1);
  SCORE_MAKE_PAIR_COLOR(Tender2);
  SCORE_MAKE_PAIR_COLOR(Tender3);
  SCORE_MAKE_PAIR_COLOR(Pulse1);
  SCORE_MAKE_PAIR_COLOR(Pulse2);

  return vec;
}

static bool pulse(QBrush& ref, bool pulse)
{
  bool invert = false;
  auto col = ref.color();
  auto alpha = col.alphaF();
  if(pulse)
  {
    alpha += 0.02;
    if(alpha >= 1)
    {
      invert = true;
      alpha = 1;
    }
    col.setAlphaF(alpha);
  }
  else
  {
    alpha -= 0.02;
    if(alpha <= 0.5)
    {
      invert = true;
      alpha = 0.5;
    }
    col.setAlphaF(alpha);
  }
  ref.setColor(col);
  return invert;
}

void Skin::timerEvent(QTimerEvent* event)
{
  pulse(Pulse1, m_pulseDirection);
  auto invert = pulse(Pulse2, m_pulseDirection);
  if(invert)
    m_pulseDirection = !m_pulseDirection;
}

const QBrush* Skin::fromString(const QString& s) const
{
  auto it = m_colorMap.left.find(s);
  return it != m_colorMap.left.end() ? it->second : nullptr;
}

QBrush* Skin::fromString(const QString& s)
{
    auto it = m_colorMap.left.find(s);
    return it != m_colorMap.left.end() ? it->second : nullptr;
}

QString Skin::toString(const QBrush* c) const
{
  auto it = m_colorMap.right.find(const_cast<QBrush*>(c));
  return it != m_colorMap.right.end() ? it->second : nullptr;
}

#undef SCORE_INSERT_COLOR
#undef SCORE_CONVERT_COLOR
#undef SCORE_MAKE_PAIR_COLOR
}
