// ************************************************************************** //
//
//  Model-view-view-model framework for large GUI applications
//
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @authors   see AUTHORS
//
// ************************************************************************** //

#ifndef QUICKSIMEDITOR_H
#define QUICKSIMEDITOR_H

#include <QWidget>

//! Quick reflectivity simulations.

class QuickSimEditor : public QWidget
{
    Q_OBJECT
public:
    QuickSimEditor(QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
};

#endif // QUICKSIMEDITOR_H
