// ZZ


#include "UI/Widget/SeptentUserWidget.h"

#include "UI/WidgetController/SeptentWidgetController.h"

void USeptentUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
