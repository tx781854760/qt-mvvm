// ************************************************************************** //
//
//  Model-view-view-model framework for large GUI applications
//
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @authors   see AUTHORS
//
// ************************************************************************** //

#include "google_test.h"
#include "qcustomplot.h"
#include <QSignalSpy>
#include <mvvm/model/sessionmodel.h>
#include <mvvm/plotting/viewportaxisplotcontroller.h>
#include <mvvm/standarditems/axisitems.h>

using namespace ModelView;

//! Testing AxisPlotControllers.

class ViewportAxisPlotControllerTest : public ::testing::Test
{
public:
    ~ViewportAxisPlotControllerTest();

    std::unique_ptr<QSignalSpy> createSpy(QCPAxis* axis)
    {
        return std::make_unique<QSignalSpy>(
            axis, static_cast<void (QCPAxis::*)(const QCPRange&)>(&QCPAxis::rangeChanged));
    }
};

ViewportAxisPlotControllerTest::~ViewportAxisPlotControllerTest() = default;

//! Initial state.

TEST_F(ViewportAxisPlotControllerTest, initialState)
{
    auto custom_plot = std::make_unique<QCustomPlot>();

    auto axis = custom_plot->xAxis;

    // checking initial defaults
    const double customplot_default_lower(0.0), customplot_default_upper(5.0);
    EXPECT_EQ(axis->range().lower, customplot_default_lower);
    EXPECT_EQ(axis->range().upper, customplot_default_upper);

    // controller shouldn''t change axis range
    ViewportAxisPlotController controller(axis);
    EXPECT_EQ(axis->range().lower, customplot_default_lower);
    EXPECT_EQ(axis->range().upper, customplot_default_upper);

    // checking axis signaling
    auto xChanged = createSpy(custom_plot->xAxis);
    auto yChanged = createSpy(custom_plot->yAxis);

    // changing range of axis
    custom_plot->xAxis->setRangeLower(1.0);

    // checking that QCPaxis properly emiting signals
    EXPECT_EQ(xChanged->count(), 1);
    EXPECT_EQ(yChanged->count(), 0);
}

//! Controller subscribed to ViewportAxisItem.
//! Checking that QCPAxis get same parameters as in AxisItem.

TEST_F(ViewportAxisPlotControllerTest, setViewportAxisItem)
{
    auto custom_plot = std::make_unique<QCustomPlot>();
    const double expected_min = 1.0;
    const double expected_max = 2.0;

    // creating the model with single ViewportAxisItem
    SessionModel model;
    auto axisItem = model.insertItem<ViewportAxisItem>();
    axisItem->setProperty(ViewportAxisItem::P_MIN, expected_min);
    axisItem->setProperty(ViewportAxisItem::P_MAX, expected_max);

    // setting up QCustomPlot and item controller.
    ASSERT_TRUE(custom_plot->xAxis != nullptr);
    ViewportAxisPlotController controller(custom_plot->xAxis);

    auto prev_y_range = custom_plot->yAxis->range();
    auto xChanged = createSpy(custom_plot->xAxis);
    auto yChanged = createSpy(custom_plot->yAxis);

    // Subscribtion to ViewportAxisItem should change QCPAxis range for X.
    controller.setItem(axisItem);

    EXPECT_EQ(custom_plot->xAxis->range().lower, expected_min);
    EXPECT_EQ(custom_plot->xAxis->range().upper, expected_max);
    EXPECT_EQ(xChanged->count(), 1);
    EXPECT_EQ(yChanged->count(), 0);

    // Range for QCPAxis y-axis should stay the same.
    EXPECT_EQ(custom_plot->yAxis->range(), prev_y_range);
}

//! Controller subscribed to ViewportAxisItem.
//! Change QCPAxis and check that ViewportAxisItem got new values.

TEST_F(ViewportAxisPlotControllerTest, changeQCPAxis)
{
    auto custom_plot = std::make_unique<QCustomPlot>();

    // creating the model with single ViewportAxisItem
    SessionModel model;
    auto axisItem = model.insertItem<ViewportAxisItem>();
    axisItem->setProperty(ViewportAxisItem::P_MIN, 42.0);
    axisItem->setProperty(ViewportAxisItem::P_MAX, 42.1);

    // setting up QCustomPlot and item controller.
    const double expected_min = 1.0;
    const double expected_max = 2.0;
    auto xChanged = createSpy(custom_plot->xAxis);
    auto yChanged = createSpy(custom_plot->yAxis);

    // Setting up controller.
    ViewportAxisPlotController controller(custom_plot->xAxis);
    controller.setItem(axisItem);

    EXPECT_EQ(xChanged->count(), 1);
    EXPECT_EQ(yChanged->count(), 0);

    // Changing QCPAxis
    custom_plot->xAxis->setRange(expected_min, expected_max);
    EXPECT_EQ(xChanged->count(), 2);
    EXPECT_EQ(yChanged->count(), 0);

    // Check changed properties in ViewportAxisItem
    EXPECT_EQ(axisItem->property<double>(ViewportAxisItem::P_MIN), expected_min);
    EXPECT_EQ(axisItem->property<double>(ViewportAxisItem::P_MAX), expected_max);
}

//! Controller subscribed to ViewportAxisItem.
//! Change ViewportAxisItem and check that QCPAxis got new values.

TEST_F(ViewportAxisPlotControllerTest, changeViewportAxisItem)
{
    auto custom_plot = std::make_unique<QCustomPlot>();

    // creating the model with single ViewportAxisItem
    SessionModel model;
    auto axisItem = model.insertItem<ViewportAxisItem>();
    axisItem->setProperty(ViewportAxisItem::P_MIN, 42.0);
    axisItem->setProperty(ViewportAxisItem::P_MAX, 42.1);

    // setting up QCustomPlot and item controller.
    ViewportAxisPlotController controller(custom_plot->xAxis);
    controller.setItem(axisItem);
    auto xChanged = createSpy(custom_plot->xAxis);
    auto yChanged = createSpy(custom_plot->yAxis);

    // changing values
    const double expected_min = 1.0;
    const double expected_max = 2.0;
    axisItem->setProperty(ViewportAxisItem::P_MIN, expected_min);
    axisItem->setProperty(ViewportAxisItem::P_MAX, expected_max);

    // Checking QCPAxis
    EXPECT_EQ(xChanged->count(), 2);
    EXPECT_EQ(yChanged->count(), 0);
    EXPECT_EQ(custom_plot->xAxis->range().lower, expected_min);
    EXPECT_EQ(custom_plot->xAxis->range().upper, expected_max);
}

//! Controller subscribed to ViewportAxisItem.
//! Change ViewportAxisItem logz and check that QCPAxis got new values.

TEST_F(ViewportAxisPlotControllerTest, changeViewportLogz)
{
    auto custom_plot = std::make_unique<QCustomPlot>();

    // creating the model with single ViewportAxisItem
    SessionModel model;
    auto axisItem = model.insertItem<ViewportAxisItem>();

    // setting up QCustomPlot and item controller.
    auto qcp_axis = custom_plot->xAxis;
    ViewportAxisPlotController controller(qcp_axis);
    controller.setItem(axisItem);

    // initial linear scale of axis
    EXPECT_EQ(qcp_axis->scaleType(), QCPAxis::stLinear);

    // changing scale
    axisItem->setProperty(ViewportAxisItem::P_IS_LOG, true);

    // QCPAxis should switch to logarithmic
    EXPECT_EQ(qcp_axis->scaleType(), QCPAxis::stLogarithmic);
}

//! Controller subscribed to ViewportAxisItem.
//! Change ViewportAxisItem and check that QCPAxis got new values.
//! Same test as before, only QCPAxis y-axis checked.

TEST_F(ViewportAxisPlotControllerTest, changeViewportAxisItemYCase)
{
    auto custom_plot = std::make_unique<QCustomPlot>();

    // creating the model with single ViewportAxisItem
    SessionModel model;
    auto axisItem = model.insertItem<ViewportAxisItem>();
    axisItem->setProperty(ViewportAxisItem::P_MIN, 42.0);
    axisItem->setProperty(ViewportAxisItem::P_MAX, 42.1);

    // setting up QCustomPlot and item controller.
    ViewportAxisPlotController controller(custom_plot->yAxis);
    controller.setItem(axisItem);
    auto xChanged = createSpy(custom_plot->xAxis);
    auto yChanged = createSpy(custom_plot->yAxis);

    // changing values
    const double expected_min = 1.0;
    const double expected_max = 2.0;
    axisItem->setProperty(ViewportAxisItem::P_MIN, expected_min);
    axisItem->setProperty(ViewportAxisItem::P_MAX, expected_max);

    // Checking QCPAxis
    EXPECT_EQ(xChanged->count(), 0);
    EXPECT_EQ(yChanged->count(), 2);
    EXPECT_EQ(custom_plot->yAxis->range().lower, expected_min);
    EXPECT_EQ(custom_plot->yAxis->range().upper, expected_max);
}

//! Model with two AxisItem's. Controller first is subscribed to one item, then to another.

TEST_F(ViewportAxisPlotControllerTest, oneControllerTwoAxisItems)
{
    auto custom_plot = std::make_unique<QCustomPlot>();

    // creating the model with single ViewportAxisItem
    SessionModel model;
    auto axis_item0 = model.insertItem<ViewportAxisItem>();
    axis_item0->setProperty(ViewportAxisItem::P_MIN, 1.0);
    axis_item0->setProperty(ViewportAxisItem::P_MAX, 2.0);

    auto axis_item1 = model.insertItem<ViewportAxisItem>();
    axis_item1->setProperty(ViewportAxisItem::P_MIN, 10.0);
    axis_item1->setProperty(ViewportAxisItem::P_MAX, 20.0);

    // setting up QCustomPlot and item controller.
    auto controller = std::make_unique<ViewportAxisPlotController>(custom_plot->xAxis);
    controller->setItem(axis_item0);
    auto xChanged = createSpy(custom_plot->xAxis);
    auto yChanged = createSpy(custom_plot->yAxis);

    // initial axis status
    EXPECT_EQ(axis_item0->property<double>(ViewportAxisItem::P_MIN),
              custom_plot->xAxis->range().lower);
    EXPECT_EQ(axis_item0->property<double>(ViewportAxisItem::P_MAX),
              custom_plot->xAxis->range().upper);

    // switching to second axis
    controller->setItem(axis_item1);

    EXPECT_EQ(xChanged->count(), 1);
    EXPECT_EQ(yChanged->count(), 0);

    EXPECT_EQ(axis_item1->property<double>(ViewportAxisItem::P_MIN),
              custom_plot->xAxis->range().lower);
    EXPECT_EQ(axis_item1->property<double>(ViewportAxisItem::P_MAX),
              custom_plot->xAxis->range().upper);

    // changing QCPAxis
    const double expected_min = 100.0;
    const double expected_max = 200.0;
    custom_plot->xAxis->setRange(expected_min, expected_max);

    // previous axis should still have original values
    EXPECT_EQ(axis_item0->property<double>(ViewportAxisItem::P_MIN), 1.0);
    EXPECT_EQ(axis_item0->property<double>(ViewportAxisItem::P_MAX), 2.0);

    // second axis should get values from QCPAxis
    EXPECT_EQ(axis_item1->property<double>(ViewportAxisItem::P_MIN), expected_min);
    EXPECT_EQ(axis_item1->property<double>(ViewportAxisItem::P_MAX), expected_max);

    // removing axes from the model
    model.removeItem(model.rootItem(), {"", 0});
    model.removeItem(model.rootItem(), {"", 0});
    EXPECT_EQ(model.rootItem()->childrenCount(), 0);

    // no UB should follow (valgrind will tell us)
    custom_plot->xAxis->setRange(1.0, 2.0);

    // destroying controller, no UB
    controller.reset();
    custom_plot->xAxis->setRange(2.0, 3.0);
}
