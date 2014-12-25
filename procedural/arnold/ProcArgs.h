#ifndef PROCARGS_H
#define PROCARGS_H

#include <ai.h>
#include <vector>

namespace VII {

	/*!
	 * \brief Procedural arguments container
	 * \note No need for to be a full fledge class, a struct is sufficient
	 */
	struct ProcArgs {
		ProcArgs();
		AtNode * proceduralNode;
		std::vector<struct AtNode *> createdNodes;
	};

}; // namespace VII

#endif // PROCARGS_H
