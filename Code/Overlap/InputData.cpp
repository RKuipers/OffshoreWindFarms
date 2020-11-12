#include "InputData.h"

YearData* YearData::getYear()
{
	return this;
}

MonthData* MonthData::getMonth()
{
	return this;
}

YearData* InputData::getYear()
{
	return nullptr;
}

MonthData* InputData::getMonth()
{
	return nullptr;
}
