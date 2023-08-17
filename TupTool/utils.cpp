#include "utils.h"

#include "tup_v1_body.h"

void initCopList(QComboBox& comboBox, tup_v1_cop_t preSelect)
{
    comboBox.clear();

    comboBox.addItem("SYN", tup_v1_cop_syn);
    comboBox.addItem("ACK", tup_v1_cop_ack);
    comboBox.addItem("DATA", tup_v1_cop_data);
    comboBox.addItem("FIN", tup_v1_cop_fin);

    comboBox.setCurrentIndex(comboBox.findData(preSelect));
}

void initErrorList(QComboBox& comboBox)
{
    comboBox.clear();

    comboBox.addItem("OK", TUP_OK);
    comboBox.addItem("LEN", TUP_ERROR_LEN);
    comboBox.addItem("UNKNOWN", TUP_ERROR_UNKNOWN);
    comboBox.addItem("NOMEMORY", TUP_ERROR_NOMEMORY);
    comboBox.addItem("CRC32", TUP_ERROR_CRC32);
}
