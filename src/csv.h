typedef enum
{
  CSV_EXPORT_FIELD_TRANSACTION_NUMBER     = 1 << 0,
  CSV_EXPORT_FIELD_TRANSACTION_MOTHER     = 1 << 1,
  CSV_EXPORT_FIELD_DATE                   = 1 << 2,
  CSV_EXPORT_FIELD_VALUE_DATE             = 1 << 3,
  CSV_EXPORT_FIELD_THIRD_PARTY            = 1 << 4,
  CSV_EXPORT_FIELD_DEBIT                  = 1 << 5,
  CSV_EXPORT_FIELD_CREDIT                 = 1 << 6,
  CSV_EXPORT_FIELD_BALANCE                = 1 << 7,
  CSV_EXPORT_FIELD_PAYEMENT_METHOD        = 1 << 8,
  CSV_EXPORT_FIELD_CONTRA_PAYEMENT_METHOD = 1 << 9,
  CSV_EXPORT_FIELD_CATEGORY               = 1 << 10,
  CSV_EXPORT_FIELD_SUB_CATEGORY           = 1 << 11,
  CSV_EXPORT_FIELD_BUDGETARY_LINE         = 1 << 12,
  CSV_EXPORT_FIELD_SUB_BUDGETARY_LINE     = 1 << 13,
  CSV_EXPORT_FIELD_NOTES                  = 1 << 14,
  CSV_EXPORT_FIELD_RECONCILE_NUMBER       = 1 << 15,
  CSV_EXPORT_FIELD_VOUCHER                = 1 << 16,
  CSV_EXPORT_FIELD_BANK_INFORMATIONS      = 1 << 17,
  CSV_EXPORT_FIELD_CHECKED                = 1 << 18,
  CSV_EXPORT_FIELD_FINANCIAL_YEAR         = 1 << 19,
  CSV_EXPORT_FIELD_CHECK_NUMBER           = 1 << 20
} CsvExportFields;


void export_accounts_to_csv ( GSList*  );

