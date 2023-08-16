#pragma once

#include <cstdint>
#include <cstdbool>

#include <QByteArray>
#include <QSharedPointer>

#include "tup_v1_body.h"

struct DataFrame
{
    DataFrame() = default;
    DataFrame(const tup_v1_data_t& src);

    uint32_t j = 0;
    tup_v1_cop_t cop = tup_v1_cop_syn;
    bool isFinal = false;
    QByteArray payload;
};

using PSynFrame = QSharedPointer<tup_v1_syn_t>;
using PFinFrame = QSharedPointer<tup_v1_fin_t>;
using PAckFrame = QSharedPointer<tup_v1_ack_t>;
using PDataFrame = QSharedPointer<DataFrame>;

Q_DECLARE_METATYPE(PSynFrame);
Q_DECLARE_METATYPE(PFinFrame);
Q_DECLARE_METATYPE(PAckFrame);
Q_DECLARE_METATYPE(PDataFrame);
