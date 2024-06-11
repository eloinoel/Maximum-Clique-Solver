#include "solve_via_vc.h"
#include "degeneracy_ordering.h"
#include "upper_bounds.h"
#include "lower_bounds.h"


//TODO:
int SolverViaVC::solve_via_vc(Graph& G)
{
    //compute degeneracy ordering and degeneracy d of G
    std::tie(degeneracy_ordering, right_neighbourhoods) = std::move(degeneracy_ordering_rN(G));
    d = degeneracy(*(degeneracy_ordering.get()), *(right_neighbourhoods.get()));

    //check lower and upper bounds
    cliqueUB = degeneracy_UB(d);
    cliqueLB = degeneracy_ordering_LB(*(degeneracy_ordering.get()), *(right_neighbourhoods.get()));

    //call `solve_via_vc_for_p` for different values of p
}

bool SolverViaVC::solve_via_vc_for_p(Graph &G, unsigned int p)
{
    //compute degeneracy ordering and degeneracy d of G

    //check lower and upper bounds

    //get candidate set D = {vi ∈ V | i ≤ n − d, rdeg(vi) ≥ d − p}

    // loop over vi ∈ D
    //  a) construct ¬G[Vi], where Vi is the right-neighborhood of vi
    //  b) if ¬G[Vi] has a vertex cover of size qi := |Vi| + p − d, return true

    // construct ¬G[Vf], where Vf = {vf , . . . , vn} and f := n − d + 1

    //if G[Vf] has a vertex cover of size qf := p − 1, return true

    // else return false
    return false;
}
