/*
 * isisApplication.h
 *
 * \date Feb 11, 2009
 * \author Thomas Pröger
 */

#ifndef ISISAPPLICATION_H_
#define ISISAPPLICATION_H_

#include "itkMacro.h"
#include <string.h>

namespace isis {

/**
 * This is our main application. We tried to implement a singleton pattern.
 */
class Application {

private:

	// our singleton instance
	static Self* m_instance;
	std::string name;

	class Guardian {
		~Guardian() {
			if (Application::m_instance != 0)
				delete Application::m_instance;
		}
	};
	friend class Guardian;	// we need access to private members

protected:

	/** Constructor - Hidden on account of the singleton pattern. */
	Application(void) {
	}
	/** Destructor - Hidden in account of the singleton pattern. */
	~Application(void) {
	}

public:

	/**
	 * \typedef
	 * The ITK default naming convention.
	 */
	typedef Application Self;

	/**
	 * \brief Access to the one and only class instance.
	 *
	 * This method returns a reference to the unique class instance,
	 * according to the singleton pattern.
	 * \param argc The number of positional parameters.
	 * \param argv A string array containing the positional parameters.
	 * \return A reference to the class' instance.
	 */
	static Self* instance(void);

	/**
	 * \brief Starts the application loop.
	 *
	 * Starts the main application loop. The programm should be terminated
	 * after the loop is finished. This function is *truly virtual so this
	 * makes this class abstract.
	 */
	virtual void start() = 0;

};

Application::Self* m_instance = 0;

inline Self *Application::instance(void) {
	if (!m_instance) {
		static Guardian g;
		m_instance = new Self;
	}
	return m_instance;
}

}

#endif /* ISISAPPLICATION_H_ */