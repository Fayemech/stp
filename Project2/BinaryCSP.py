from collections import deque

"""
	Base class for unary constraints
	Implement isSatisfied in subclass to use
"""
class UnaryConstraint:
	def __init__(self, var):
		self.var = var

	def isSatisfied(self, value):
		util.raiseNotDefined()

	def affects(self, var):
		return var == self.var


"""	
	Implementation of UnaryConstraint
	Satisfied if value does not match passed in paramater
"""
class BadValueConstraint(UnaryConstraint):
	def __init__(self, var, badValue):
		self.var = var
		self.badValue = badValue

	def isSatisfied(self, value):
		return not value == self.badValue

	def __repr__(self):
		return 'BadValueConstraint (%s) {badValue: %s}' % (str(self.var), str(self.badValue))


"""	
	Implementation of UnaryConstraint
	Satisfied if value matches passed in paramater
"""
class GoodValueConstraint(UnaryConstraint):
	def __init__(self, var, goodValue):
		self.var = var
		self.goodValue = goodValue

	def isSatisfied(self, value):
		return value == self.goodValue

	def __repr__(self):
		return 'GoodValueConstraint (%s) {goodValue: %s}' % (str(self.var), str(self.goodValue))


"""
	Base class for binary constraints
	Implement isSatisfied in subclass to use
"""
class BinaryConstraint:
	def __init__(self, var1, var2):
		self.var1 = var1
		self.var2 = var2

	def isSatisfied(self, value1, value2):
		util.raiseNotDefined()

	def affects(self, var):
		return var == self.var1 or var == self.var2

	def otherVariable(self, var):
		if var == self.var1:
			return self.var2
		return self.var1


"""
	Implementation of BinaryConstraint
	Satisfied if both values assigned are different
"""
class NotEqualConstraint(BinaryConstraint):
	def isSatisfied(self, value1, value2):
		if value1 == value2:
			return False
		return True

	def __repr__(self):
	    return 'BadValueConstraint (%s, %s)' % (str(self.var1), str(self.var2))

'''
for the extra queen problem
'''

class fourqueenConstraint(BinaryConstraint) :
    def isSatisfied(self, value1, value2) :
        if value1 == None :
            return False;
        elif value2 == None :
            return False;
        V1 = V2 = 0;
        if value1 == 'A' :
            V1 = 0;
        elif value1 == 'B' :
            V1 = 1;
        elif value1 == 'C' :
            V1 = 2;
        else :
            V1 = 3;
        if value2 == 'A' :
            V2 = 0;
        elif value2 == 'B' :
            V2 = 1;
        elif value2 == 'C' :
            V2 = 2;
        else :
            V2 = 3;
        if value1 != value2 and abs(int(self.var1) - int(self.var2)) != abs(V1 - V2) :
            return True;
        return False;

class ConstraintSatisfactionProblem:
	"""
	Structure of a constraint satisfaction problem.
	Variables and domains should be lists of equal length that have the same order.
	varDomains is a dictionary mapping variables to possible domains.

	Args:
		variables (list<string>): a list of variable names
		domains (list<set<value>>): a list of sets of domains for each variable
		binaryConstraints (list<BinaryConstraint>): a list of binary constraints to satisfy
		unaryConstraints (list<BinaryConstraint>): a list of unary constraints to satisfy
	"""
	def __init__(self, variables, domains, binaryConstraints = [], unaryConstraints = []):
		self.varDomains = {}
		for i in xrange(len(variables)):
			self.varDomains[variables[i]] = domains[i]
		self.binaryConstraints = binaryConstraints
		self.unaryConstraints = unaryConstraints

	def __repr__(self):
	    return '---Variable Domains\n%s---Binary Constraints\n%s---Unary Constraints\n%s' % ( \
	        ''.join([str(e) + ':' + str(self.varDomains[e]) + '\n' for e in self.varDomains]), \
	        ''.join([str(e) + '\n' for e in self.binaryConstraints]), \
	        ''.join([str(e) + '\n' for e in self.binaryConstraints]))


class Assignment:
	"""
	Representation of a partial assignment.
	Has the same varDomains dictionary stucture as ConstraintSatisfactionProblem.
	Keeps a second dictionary from variables to assigned values, with None being no assignment.

	Args:
		csp (ConstraintSatisfactionProblem): the problem definition for this assignment
	"""
	def __init__(self, csp):
		self.varDomains = {}
		for var in csp.varDomains:
			self.varDomains[var] = set(csp.varDomains[var])
		self.assignedValues = { var: None for var in self.varDomains }

	"""
	Determines whether this variable has been assigned.

	Args:
		var (string): the variable to be checked if assigned
	Returns:
		boolean
		True if var is assigned, False otherwise
	"""
	def isAssigned(self, var):
		return self.assignedValues[var] != None

	"""
	Determines whether this problem has all variables assigned.

	Returns:
		boolean
		True if assignment is complete, False otherwise
	"""
	def isComplete(self):
		for var in self.assignedValues:
			if not self.isAssigned(var):
				return False
		return True

	"""
	Gets the solution in the form of a dictionary.

	Returns:
		dictionary<string, value>
		A map from variables to their assigned values. None if not complete.
	"""
	def extractSolution(self):
		if not self.isComplete():
			return None
		return self.assignedValues

	def __repr__(self):
	    return '---Variable Domains\n%s---Assigned Values\n%s' % ( \
	        ''.join([str(e) + ':' + str(self.varDomains[e]) + '\n' for e in self.varDomains]), \
	        ''.join([str(e) + ':' + str(self.assignedValues[e]) + '\n' for e in self.assignedValues]))



####################################################################################################


"""
	Checks if a value assigned to a variable is consistent with all binary constraints in a problem.
	Do not assign value to var. Only check if this value would be consistent or not.
	If the other variable for a constraint is not assigned, then the new value is consistent with the constraint.

	Args:
		assignment (Assignment): the partial assignment
		csp (ConstraintSatisfactionProblem): the problem definition
		var (string): the variable that would be assigned
		value (value): the value that would be assigned to the variable
	Returns:
		boolean
		True if the value would be consistent with all currently assigned values, False otherwise
"""
def consistent(assignment, csp, var, value):
    for con in csp.binaryConstraints :
        if (con.affects(var)) :
            if (assignment.assignedValues[con.otherVariable(var)] != None) :
                if (not con.isSatisfied(value, assignment.assignedValues[con.otherVariable(var)])) :
                    return False;
    return True;

"""
	Recursive backtracking algorithm.
	A new assignment should not be created. The assignment passed in should have its domains updated with inferences.
	In the case that a recursive call returns failure or a variable assignment is incorrect, the inferences made along
	the way should be reversed. See maintainArcConsistency and forwardChecking for the format of inferences.

	Examples of the functions to be passed in:
	orderValuesMethod: orderValues, leastConstrainingValuesHeuristic
	selectVariableMethod: chooseFirstVariable, minimumRemainingValuesHeuristic
	inferenceMethod: noInferences, maintainArcConsistency, forwardChecking

	Args:
		assignment (Assignment): a partial assignment to expand upon
		csp (ConstraintSatisfactionProblem): the problem definition
		orderValuesMethod (function<assignment, csp, variable> returns list<value>): a function to decide the next value to try
		selectVariableMethod (function<assignment, csp> returns variable): a function to decide which variable to assign next
	Returns:
		Assignment
		A completed and consistent assignment. None if no solution exists.
"""
def recursiveBacktracking(assignment, csp, orderValuesMethod, selectVariableMethod):
    if (assignment.isComplete()) :
        return assignment;
    
    var = selectVariableMethod(assignment,csp);
    
    if (var == None) :
        return None;
    
    values = orderValuesMethod(assignment, csp, var);
    
    for value in values :
        if consistent(assignment, csp, var, value) :
            assignment.assignedValues[var] = value;
            if (recursiveBacktracking(assignment, csp, orderValuesMethod, selectVariableMethod)) :
                return recursiveBacktracking(assignment, csp, orderValuesMethod, selectVariableMethod);
        assignment.assignedValues[var] = None;
        
    return None;

    



"""
	Uses unary constraints to eleminate values from an assignment.

	Args:
		assignment (Assignment): a partial assignment to expand upon
		csp (ConstraintSatisfactionProblem): the problem definition
	Returns:
		Assignment
		An assignment with domains restricted by unary constraints. None if no solution exists.
"""
def eliminateUnaryConstraints(assignment, csp):
	domains = assignment.varDomains
	for var in domains:
		for constraint in (c for c in csp.unaryConstraints if c.affects(var)):
			for value in (v for v in list(domains[var]) if not constraint.isSatisfied(v)):
				domains[var].remove(value)
				if len(domains[var]) == 0:
				 	# Failure due to invalid assignment
				 	return None
	return assignment


"""
	Trivial method for choosing the next variable to assign.
	Uses no heuristics.
"""
def chooseFirstVariable(assignment, csp):
	for var in csp.varDomains:
		if not assignment.isAssigned(var):
			return var


"""
	Selects the next variable to try to give a value to in an assignment.
	Uses minimum remaining values heuristic to pick a variable. Use degree heuristic for breaking ties.

	Args:
		assignment (Assignment): the partial assignment to expand
		csp (ConstraintSatisfactionProblem): the problem description
	Returns:
		the next variable to assign
"""
def minimumRemainingValuesHeuristic(assignment, csp):
    bestvar = None;
    domains = assignment.varDomains;
    
    for first in assignment.assignedValues :
        if not assignment.isAssigned(first) :
            bestvar = first;
            break;
    
    for var in assignment.assignedValues :
        if not assignment.isAssigned(var) :
            if var != bestvar :
                if len(domains[var]) < len(domains[bestvar]) :
                    bestvar = var;
                elif len(domains[var]) == len(domains[bestvar]) :
                    cnt = bestcnt = 0;
                    for con in csp.binaryConstraints :
                        if con.affects(var) :
                            cnt += 1;
                        if con.affects(bestvar) :
                            bestcnt += 1;
                    if cnt > bestcnt :
                        bestvar = var;
                        
    return bestvar;
                     
                    

"""
	Trivial method for ordering values to assign.
	Uses no heuristics.
"""

def orderValues(assignment, csp, var):
	return list(assignment.varDomains[var])


"""
	Creates an ordered list of the remaining values left for a given variable.
	Values should be attempted in the order returned.
	The least constraining value should be at the front of the list.

	Args:
		assignment (Assignment): the partial assignment to expand
		csp (ConstraintSatisfactionProblem): the problem description
		var (string): the variable to be assigned the values
	Returns:
		list<values>
		a list of the possible values ordered by the least constraining value heuristic
"""
def leastConstrainingValuesHeuristic(assignment, csp, var):
    values = list(assignment.varDomains[var]);
    pair = [];
    res = [];
    
    for value in values :
        cnt = usefulcount(assignment, csp, var, value);
        if len(pair) == 0 :
            pair.append((value, cnt));
            res.append(value);
        else :
            index = 0;
            while (index < len(pair) and cnt < pair[index][1]) :
                index += 1;
            pair.insert(index, (value, cnt));
            res.insert(index, value);
            
    return res;
        
    
 
def usefulcount(assignment, csp, var, value) :
    cnt = 0
    if not consistent(assignment, csp, var, value) :
		for con in csp.binaryConstraints :
			if con.affects(var) :
				if assignment.assignedValues[con.otherVariable(var)] == value :
					cnt -= 1;
		return cnt;

    '''
    for var1 in assignment.assignedValues :
        if assignment.assignedValues[var1] != None :
            valuepair[var1] = assignment.assignedValues[var1];
    
    valuepair[var] = value;
    '''
    
    
    assignment.assignedValues[var] = value;
    for con in csp.binaryConstraints:
		if con.affects(var):
			othervar = con.otherVariable(var)
			if not assignment.isAssigned(othervar):
				for otherval in assignment.varDomains[othervar]:
					consistence = True
					for con1 in csp.binaryConstraints:
						if con1.affects(othervar) and not con1.isSatisfied(otherval, assignment.assignedValues[con1.otherVariable(othervar)]):
							consistence = False
					if consistence:
						cnt += 1
    assignment.assignedValues[var] = None;
    return cnt

        

"""
	Trivial method for making no inferences.
"""
def noInferences(assignment, csp, var, value):
	return set([])


"""
	Implements the forward checking algorithm.
	Each inference should take the form of (variable, value) where the value is being removed from the
	domain of variable. This format is important so that the inferences can be reversed if they
	result in a conflicting partial assignment. If the algorithm reveals an inconsistency, any
	inferences made should be reversed before ending the fuction.

	Args:
		assignment (Assignment): the partial assignment to expand
		csp (ConstraintSatisfactionProblem): the problem description
		var (string): the variable that has just been assigned a value
		value (string): the value that has just been assigned
	Returns:
		set<tuple<variable, value>>
		the inferences made in this call or None if inconsistent assignment
"""
def forwardChecking(assignment, csp, var, value):
    inferences = set([]);
    domains = assignment.varDomains;
    
    for con in csp.binaryConstraints :
        if con.affects(var) :
            othervar = con.otherVariable(var);
            localdomain = assignment.varDomains[othervar];
            localdomain = list(localdomain)
            for val in localdomain :
                if not con.isSatisfied(value, val) :
                    domains[othervar].remove(val);
                    inferences.add((othervar, val));
                if len(domains[othervar]) == 0 :
                    for x in inferences :
                        domains[x[0]].add(x[1]);
                    return None;
                
    return inferences;
    

"""
	Recursive backtracking algorithm.
	A new assignment should not be created. The assignment passed in should have its domains updated with inferences.

	In the case that a recursive call returns failure or a variable assignment is incorrect, the inferences made along
	the way should be reversed. See maintainArcConsistency and forwardChecking for the format of inferences.
1`

	Examples of the functions to be passed in:
	orderValuesMethod: orderValues, leastConstrainingValuesHeuristic
	selectVariableMethod: chooseFirstVariable, minimumRemainingValuesHeuristic
	inferenceMethod: noInferences, maintainArcConsistency, forwardChecking


	Args:
		assignment (Assignment): a partial assignment to expand upon
		csp (ConstraintSatisfactionProblem): the problem definition
		orderValuesMethod (function<assignment, csp, variable> returns list<value>): a function to decide the next value to try
		selectVariableMethod (function<assignment, csp> returns variable): a function to decide which variable to assign next
		inferenceMethod (function<assignment, csp, variable, value> returns set<variable, value>): a function to specify what type of inferences to use
				Can be forwardChecking or maintainArcConsistency
	Returns:
		Assignment

		A completed and consistent assignment. None if no solution exists.
"""
def recursiveBacktrackingWithInferences(assignment, csp, orderValuesMethod, selectVariableMethod, inferenceMethod):
    if (assignment.isComplete()) :
        return assignment;
    
    var = selectVariableMethod(assignment,csp);
    
    if (var == None) :
        return None;
    
    values = orderValuesMethod(assignment, csp, var);
    
    for value in values :
        if consistent(assignment, csp, var, value) :
            assignment.assignedValues[var] = value;
            inferences = inferenceMethod(assignment, csp, var, value)
            if (recursiveBacktrackingWithInferences(assignment, csp, orderValuesMethod, selectVariableMethod, inferenceMethod)) :
                return recursiveBacktrackingWithInferences(assignment, csp, orderValuesMethod, selectVariableMethod, inferenceMethod);
            assignment.assignedValues[var] = None;
            if inferences != None:
                for x in inferences :
                    assignment.varDomains[x[0]].add(x[1]);
                    
    return None;



"""
	Helper funciton to maintainArcConsistency and AC3.
	Remove values from var2 domain if constraint cannot be satisfied.
	Each inference should take the form of (variable, value) where the value is being removed from the
	domain of variable. This format is important so that the inferences can be reversed if they
	result in a conflicting partial assignment. If the algorithm reveals an inconsistency, any
	inferences made should be reversed before ending the fuction.

	Args:
		assignment (Assignment): the partial assignment to expand
		csp (ConstraintSatisfactionProblem): the problem description
		var1 (string): the variable with consistent values
		var2 (string): the variable that should have inconsistent values removed
		constraint (BinaryConstraint): the constraint connecting var1 and var2
	Returns:
		set<tuple<variable, value>>
		the inferences made in this call or None if inconsistent assignment
"""
def revise(assignment, csp, var1, var2, constraint):
    inferences = set([]);
    for val2 in assignment.varDomains[var2]:
		sign = False
		for val1 in assignment.varDomains[var1]:
			if constraint.isSatisfied(val2, val1):
				sign = True;
		if not sign:
			inferences.add((var2, val2));
					
    for x in inferences:
		assignment.varDomains[x[0]].remove(x[1]);

    if not assignment.varDomains[var2]:
		for x in inferences:
			assignment.varDomains[x[0]].add(x[1]);
		return None
    
    return inferences
    
        


"""
	Implements the maintaining arc consistency algorithm.
	Inferences take the form of (variable, value) where the value is being removed from the
	domain of variable. This format is important so that the inferences can be reversed if they
	result in a conflicting partial assignment. If the algorithm reveals an inconsistency, and
	inferences made should be reversed before ending the fuction.

	Args:
		assignment (Assignment): the partial assignment to expand
		csp (ConstraintSatisfactionProblem): the problem description
		var (string): the variable that has just been assigned a value
		value (string): the value that has just been assigned
	Returns:
		set<<variable, value>>
		the inferences made in this call or None if inconsistent assignment
"""
def maintainArcConsistency(assignment, csp, var, value):
    inferences = set([]);
    q = deque();
    
    for con in csp.binaryConstraints :
        if con.affects(var) :
            q.append([var, con.otherVariable(var), con]);
    
    while len(q) != 0 :
        x = q.popleft();
        old = x[0];
        revised = revise(assignment, csp, x[0], x[1], x[2]);
        if revised != None and len(revised) != 0 :
            inferences.update(revised);
            for con in csp.binaryConstraints :
                if con.affects(x[1]) and con.otherVariable(x[1]) != old :
                    q.append([x[1], con.otherVariable(x[1]), con]);
        elif revised == None:
            for x in inferences :
                assignment.varDomains[x[0]].add(x[1]);
            return None;
    return inferences;

	



"""
	AC3 algorithm for constraint propogation. Used as a preprocessing step to reduce the problem
	before running recursive backtracking.

	Args:
		assignment (Assignment): the partial assignment to expand
		csp (ConstraintSatisfactionProblem): the problem description
	Returns:
		Assignment
		the updated assignment after inferences are made or None if an inconsistent assignment
"""
def AC3(assignment, csp):
    inferences = set([]);
    q = deque();
    for var in assignment.assignedValues :
        for con in csp.binaryConstraints :
            if con.affects(var) :
                q.append([var, con.otherVariable(var), con]);
                
    while len(q) != 0 :
        x = q.popleft();
        old = x[0];
        revised = revise(assignment, csp, x[0], x[1], x[2]);
        if revised != None and len(revised) != 0 :
            inferences.update(revised);
            for con in csp.binaryConstraints :
                if con.affects(x[1]) and con.otherVariable(x[1]) != old :
                    q.append([x[1], con.otherVariable(x[1]), con]);
        elif revised == None:
            for x in inferences :
                assignment.varDomains[x[0]].add(x[1]);
            return None;
    return assignment;


"""
	Solves a binary constraint satisfaction problem.

	Args:
		csp (ConstraintSatisfactionProblem): a CSP to be solved
		orderValuesMethod (function): a function to decide the next value to try
		selectVariableMethod (function): a function to decide which variable to assign next
		inferenceMethod (function): a function to specify what type of inferences to use
		useAC3 (boolean): specifies whether to use the AC3 preprocessing step or not
	Returns:
		dictionary<string, value>
		A map from variables to their assigned values. None if no solution exists.
"""
def solve(csp, orderValuesMethod=leastConstrainingValuesHeuristic, selectVariableMethod=minimumRemainingValuesHeuristic, inferenceMethod=None, useAC3=True):
	assignment = Assignment(csp)

	assignment = eliminateUnaryConstraints(assignment, csp)
	if assignment == None:
		return assignment

	if useAC3:
		assignment = AC3(assignment, csp)
		if assignment == None:
			return assignment
	if inferenceMethod is None or inferenceMethod==noInferences:
		assignment = recursiveBacktracking(assignment, csp, orderValuesMethod, selectVariableMethod)
	else:
		assignment = recursiveBacktrackingWithInferences(assignment, csp, orderValuesMethod, selectVariableMethod, inferenceMethod)
	if assignment == None:
		return assignment

	return assignment.extractSolution()
