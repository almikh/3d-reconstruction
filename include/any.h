#ifndef ANY_H_INCLUDED__
#define ANY_H_INCLUDED__

#include <QtGlobal> // Q_ASSERT
#include <algorithm>
#include <typeinfo>

class any {
protected:
	struct IHolder {
		virtual ~IHolder() {}

		virtual const std::type_info& type() const = 0;
		virtual IHolder* clone() const = 0;
	};

	template<class ValueType>
	struct Holder : public IHolder {
		Holder& operator=(const Holder& holder);

	public:
		ValueType data;
		Holder(const ValueType& data) : data(data) {}

		const std::type_info& type() const {
			return typeid(ValueType);
		}

		IHolder* clone() const {
			return new Holder(data);
		}
	};

protected:
	IHolder* content_;

public:
	any();
	any(const any& other);

	template<class ValueType>
	any(const ValueType& value) : content_(new Holder<ValueType>(value)) {
	
	}

	~any(void);

	any& swap(any& other);

	any& operator=(const any& rhs);

	template<class ValueType>
	any& operator=(const ValueType& rhs) {
		any(rhs).swap(*this);
		return *this;
	}

	bool empty() const;

	void clear();

	const std::type_info& type() const;

	template<class ValueType>
	ValueType& castTo() {
		Holder<ValueType>* holder = dynamic_cast<Holder<ValueType>*>(content_);
		if (holder) return holder->data;
		
		Q_ASSERT_X(holder != nullptr, "any", "error cast to " + typeid(ValueType).name());
		return holder->mData;
	}
};

#endif
