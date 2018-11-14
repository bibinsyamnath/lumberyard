/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* DIStribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include <AzQtComponents/Components/Widgets/ColorPicker/PaletteCardCollection.h>
#include <AzQtComponents/Components/Widgets/ColorPicker/ColorController.h>
#include <AzQtComponents/Components/Widgets/ColorPicker/PaletteCard.h>
#include <AzQtComponents/Components/Widgets/ColorPicker/PaletteView.h>
#include <QVBoxLayout>

namespace AzQtComponents
{

PaletteCardCollection::PaletteCardCollection(Internal::ColorController* colorController, QUndoStack* undoStack, QWidget* parent)
    : QWidget(parent)
    , m_colorController(colorController)
    , m_undoStack(undoStack)
    , m_layout(new QVBoxLayout(this))
    , m_swatchSize({ 16, 16 })
{
    m_layout->setContentsMargins(0, 0, 0, 0);
}

PaletteCardCollection::~PaletteCardCollection()
{
    for (QSharedPointer<PaletteCard> card : m_paletteCards)
    {
        m_layout->removeWidget(card.data());
        card->setParent(nullptr);
    }
}

QSharedPointer<PaletteCard> PaletteCardCollection::makeCard(QSharedPointer<Palette> palette, const QString& title)
{
    auto card = QSharedPointer<PaletteCard>::create(palette, m_colorController, m_undoStack, this);
    card->setTitle(uniquePaletteName(card, title));
    card->setSwatchSize(m_swatchSize);
    card->setGammaEnabled(m_gammaEnabled);
    card->setGamma(m_gamma);

    return card;
}

void PaletteCardCollection::addCard(QSharedPointer<PaletteCard> card)
{
    connect(card.data(), &PaletteCard::removeClicked, this, [this, card] {
        emit removePaletteClicked(card);
    });
    connect(card.data(), &PaletteCard::saveClicked, this, [this, card] {
        emit savePaletteClicked(card);
    });
    connect(card.data(), &PaletteCard::colorSelected, m_colorController, &Internal::ColorController::setColor);

    m_layout->addWidget(card.data());
    m_paletteCards.append(card);

    emit paletteCountChanged();
}

void PaletteCardCollection::removeCard(QSharedPointer<PaletteCard> card)
{
    auto target = std::find(m_paletteCards.begin(), m_paletteCards.end(), card);

    if (target != m_paletteCards.end())
    {
        m_paletteCards.erase(target);
        m_layout->removeWidget(card.data());
        card->setParent(nullptr);
    }

    emit paletteCountChanged();
}

bool PaletteCardCollection::containsCard(QSharedPointer<PaletteCard> card) const
{
    return m_paletteCards.contains(card);
}

void PaletteCardCollection::setSwatchSize(const QSize& size)
{
    if (size == m_swatchSize)
    {
        return;
    }

    m_swatchSize = size;

    for (auto card : m_paletteCards)
    {
        card->setSwatchSize(size);
    }
}

void PaletteCardCollection::setGammaEnabled(bool enabled)
{
    if (enabled == m_gammaEnabled)
    {
        return;
    }

    m_gammaEnabled = enabled;

    for (auto card : m_paletteCards)
    {
        card->setGammaEnabled(enabled);
    }
}

void PaletteCardCollection::setGamma(qreal gamma)
{
    if (qFuzzyCompare(gamma, m_gamma))
    {
        return;
    }

    m_gamma = gamma;

    for (auto card : m_paletteCards)
    {
        card->setGamma(gamma);
    }
}

int PaletteCardCollection::count() const
{
    return m_paletteCards.count();
}

QString PaletteCardCollection::uniquePaletteName(QSharedPointer<PaletteCard> card, const QString& name) const
{
    const auto paletteNameExists = [this, card](const QString& name)
    {
        auto it = std::find_if(m_paletteCards.begin(), m_paletteCards.end(),
                               [&name](QSharedPointer<const PaletteCard> card)
                               { return name == card->title(); });
        return it != m_paletteCards.end();
    };

    if (!paletteNameExists(name))
    {
        return name;
    }

    auto lastDigit = std::find_if(name.rbegin(), name.rend(),
                                  [](const QChar ch) { return !ch.isDigit(); });
    const QString baseName = name.left(std::distance(name.begin(), lastDigit.base()));

    int lastNumber = 1;
    while (true)
    {
        QString newName = QStringLiteral("%1%2").arg(baseName).arg(lastNumber);
        if (!paletteNameExists(newName))
        {
            return newName;
        }
        ++lastNumber;
    }

    Q_UNREACHABLE();
}

} // namespace AzQtComponents
#include <Components/Widgets/ColorPicker/PaletteCardCollection.moc>