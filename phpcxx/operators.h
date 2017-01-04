#ifndef PHPCXX_OPERATORS_H
#define PHPCXX_OPERATORS_H

namespace phpcxx {
class Value;

Value operator~(const Value& op);
Value operator!(const Value& op);
bool operator==(const Value& lhs, const Value& rhs);
bool operator< (const Value& lhs, const Value& rhs);
bool operator<=(const Value& lhs, const Value& rhs);
static inline bool operator!=(const Value& lhs, const Value& rhs) { return !(lhs == rhs); }
static inline bool operator> (const Value& lhs, const Value& rhs) { return   rhs <  lhs;  }
static inline bool operator>=(const Value& lhs, const Value& rhs) { return   rhs <= lhs;  }

Value pow(Value lhs, const Value& rhs);
Value concat(Value lhs, const Value& rhs);
int compare(const Value& lhs, const Value& rhs);

}

#endif /* PHPCXX_OPERATORS_H */
