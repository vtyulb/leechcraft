/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "plotitem.h"
#include <cmath>
#include <limits>
#include <QStyleOption>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_grid.h>

Q_DECLARE_METATYPE (QList<QPointF>)

namespace LeechCraft
{
namespace Util
{
	PlotItem::PlotItem (QDeclarativeItem *parent)
	: QDeclarativeItem { parent }
	, Color_ { "#FF4B10" }
	{
		setFlag (QGraphicsItem::ItemHasNoContents, false);
	}

	QList<QPointF> PlotItem::GetPoints () const
	{
		return Points_;
	}

	void PlotItem::SetPoints (const QList<QPointF>& pts)
	{
		if (pts == Points_)
			return;

		Points_ = pts;
		emit pointsChanged ();
		update ();
	}

	double PlotItem::GetMinYValue () const
	{
		return MinYValue_;
	}

	void PlotItem::SetMinYValue (double val)
	{
		SetNewValue (val, MinYValue_, [this] { emit minYValueChanged (); });
	}

	double PlotItem::GetMaxYValue () const
	{
		return MaxYValue_;
	}

	void PlotItem::SetMaxYValue (double val)
	{
		SetNewValue (val, MaxYValue_, [this] { emit maxYValueChanged (); });
	}

	bool PlotItem::GetYGridEnabled () const
	{
		return YGridEnabled_;
	}

	void PlotItem::SetYGridEnabled (bool val)
	{
		SetNewValue (val, YGridEnabled_, [this] { emit yGridChanged (); });
	}

	bool PlotItem::GetYMinorGridEnabled () const
	{
		return YMinorGridEnabled_;
	}

	void PlotItem::SetYMinorGridEnabled (bool val)
	{
		SetNewValue (val, YMinorGridEnabled_, [this] { emit yMinorGridChanged (); });
	}

	QColor PlotItem::GetColor () const
	{
		return Color_;
	}

	void PlotItem::SetColor (const QColor& color)
	{
		SetNewValue (color, Color_, [this] { emit colorChanged (); });
	}

	bool PlotItem::GetLeftAxisEnabled () const
	{
		return LeftAxisEnabled_;
	}

	void PlotItem::SetLeftAxisEnabled (bool enabled)
	{
		SetNewValue (enabled, LeftAxisEnabled_, [this] { emit leftAxisEnabledChanged (); });
	}

	bool PlotItem::GetBottomAxisEnabled () const
	{
		return BottomAxisEnabled_;
	}

	void PlotItem::SetBottomAxisEnabled (bool enabled)
	{
		SetNewValue (enabled, BottomAxisEnabled_, [this] { emit bottomAxisEnabledChanged (); });
	}

	QString PlotItem::GetLeftAxisTitle () const
	{
		return LeftAxisTitle_;
	}

	void PlotItem::SetLeftAxisTitle (const QString& title)
	{
		SetNewValue (title, LeftAxisTitle_, [this] { emit leftAxisTitleChanged (); });
	}

	QString PlotItem::GetBottomAxisTitle () const
	{
		return BottomAxisTitle_;
	}

	void PlotItem::SetBottomAxisTitle (const QString& title)
	{
		SetNewValue (title, BottomAxisTitle_, [this] { emit bottomAxisTitleChanged (); });
	}

	void PlotItem::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*)
	{
		QwtPlot plot;
		plot.enableAxis (QwtPlot::yLeft, LeftAxisEnabled_);
		plot.enableAxis (QwtPlot::xBottom, BottomAxisEnabled_);
		plot.setAxisTitle (QwtPlot::yLeft, LeftAxisTitle_);
		plot.setAxisTitle (QwtPlot::xBottom, BottomAxisTitle_);
		plot.resize (option->rect.size ());

		plot.setAxisAutoScale (QwtPlot::xBottom, false);
		plot.setAxisScale (QwtPlot::xBottom, 0, Points_.size ());

		if (MinYValue_ < MaxYValue_)
		{
			plot.setAxisAutoScale (QwtPlot::yLeft, false);
			plot.setAxisScale (QwtPlot::yLeft, MinYValue_, MaxYValue_);
		}
		plot.setAutoFillBackground (false);
		plot.setCanvasBackground (Qt::transparent);

		if (YGridEnabled_)
		{
			auto grid = new QwtPlotGrid;
			grid->enableYMin (YMinorGridEnabled_);
			grid->enableX (false);
#if QWT_VERSION >= 0x060100
			grid->setMinorPen (QPen (Qt::gray, 1, Qt::DashLine));
#else
			grid->setMinPen (QPen (Qt::gray, 1, Qt::DashLine));
#endif
			grid->attach (&plot);
		}

		QwtPlotCurve curve;

		curve.setPen (QPen (Color_));
		auto transpColor = Color_;
		transpColor.setAlpha (20);
		curve.setBrush (transpColor);

		curve.setRenderHint (QwtPlotItem::RenderAntialiased);
		curve.attach (&plot);

		curve.setSamples (Points_.toVector ());
		plot.replot ();

		QwtPlotRenderer {}.render (&plot, painter, option->rect);
	}

	template<typename T>
	void PlotItem::SetNewValue (T val, T& ourVal, const std::function<void ()>& notifier)
	{
		if (val == ourVal)
			return;

		ourVal = val;
		notifier ();
		update ();
	}
}
}
