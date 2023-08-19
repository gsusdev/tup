#pragma once

#include <QComboBox>
#include <QIODevice>

#include "tup_v1_body.h"

void initCopList(QComboBox& comboBox, tup_v1_cop_t preSelect);
void initErrorList(QComboBox& comboBox);
