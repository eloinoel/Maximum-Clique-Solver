#include "desk.h"

/* technically an aternative is more general than just a desk, but there are no other ones known that can be found fast */
class VC_alternative : public VC_Transformation{
public:
    vector<Vertex*> A; 
    vector<Vertex*> B;

	vector<Vertex*> NA_in_NB;
	vector<Vertex*> NB_dif_A;
	vector<Vertex*> NA_dif_B;
    
    // this pattern by aleksander of executing the rule while creating the operation is pretty neat I think
    VC_alternative(vector<Vertex*> A, vector<Vertex*> B, Graph& G)
     : A(A), B(B) {

	G.transform_access.push_back(this);
	G.new_timestamp();

	assert(A.size() == B.size());

    G.new_timestamp();
    // N(A) intersection N(B)
    //std::vector<Vertex*> NAinB;

    for(Vertex* a :  A){
        for(Vertex* n : a->neighbors){
            n->marked = G.timestamp;
        }
    }

    for(Vertex* b: B){
        for(Vertex* n : b->neighbors){
            if(n->marked == G.timestamp){
                n->marked = G.timestamp-1;
                NA_in_NB.push_back(n);
            }
        }
    }

    // select all common neighbors
    for(Vertex* v: NA_in_NB){
		assert(deg(v) > 0);
        G.MM_vc_add_vertex(v);
    }
    G.new_timestamp();
    // N(B) \ A
   // std::vector<Vertex*> NBdifA;

    for (Vertex *a: A) {
			a->marked = G.timestamp;
	}
	for (Vertex *b: B) {
		for (Vertex* n: b->neighbors) {
			if (n->marked != G.timestamp) {
				n->marked = G.timestamp;
				NB_dif_A.push_back(n);
			}
		}
	}
	
    G.new_timestamp();

    // N(A) \ B
    //std::vector<Vertex*> NAdifB;

    for (Vertex *b: B) {
			b->marked = G.timestamp;
	}
	for (Vertex *a: A) {
		for (Vertex* n: a->neighbors) {
			if (n->marked != G.timestamp) {
				n->marked = G.timestamp;
				NA_dif_B.push_back(n);
			}
		}
	}

    G.new_timestamp();

      // remove A, B
	for (Vertex* a: A){
		assert(a->status == UNKNOWN);
		G.delete_vertex(a);
    }
	for (Vertex* b: B){
		assert(b->status == UNKNOWN);
		G.delete_vertex(b);
    }

    // connect N(B) \ A and N(A) \ B
	for (Vertex *a: NA_dif_B) {
		assert(a->status == UNKNOWN);
		for (Vertex* n: a->neighbors) {
			n->marked = G.timestamp;
		}

		for (Vertex *b: NB_dif_A) {
			if (b->marked != G.timestamp){
				assert(b->status == UNKNOWN);
			    G.MM_add_edge(a, b);
            }
            
		}
        G.new_timestamp();
	}

    
    int add = static_cast<int> (A.size()); // NAinB was selected, thus already added
  
    G.sol_size += add;

    //G.timestamp = time; 
	}

    void undo (Graph* G) const{
		for(auto itb = B.rbegin(); itb != B.rend(); itb++)
			G->restore_vertex(*itb);
		for(auto ita = A.rbegin(); ita != A.rend(); ita++)
			G->restore_vertex(*ita);
		G->transform_access.pop_back();
        G->sol_size -= static_cast<int> (A.size());
    }

    void resolve(Graph* G, bool partial) {
		if(partial){
			for(Vertex* a : A)
				assert(a->data.resolved_status == UNKNOWN);
			for(Vertex* b : B)
				assert(b->data.resolved_status == UNKNOWN);
			return;
		}
		size_t count_NA_dif_B = 0;
		size_t count_NB_dif_A = 0;
		
		size_t unknown = 0;

		for(Vertex* na : NA_dif_B){
			count_NA_dif_B += (na->data.resolved_status == VC);
			unknown 	   += (na->data.resolved_status == UNKNOWN);
		}
		
		for(Vertex* nb : NB_dif_A){
			count_NB_dif_A += (nb->data.resolved_status == VC);
			unknown 	   += (nb->data.resolved_status == UNKNOWN);
		}

		if(unknown > 0 && partial)
			return;

		if(count_NA_dif_B == NA_dif_B.size()){
			for(Vertex* a : A)
				a->data.resolved_status = EXCLUDED;
			for(Vertex* b : B)
				b->data.resolved_status = VC;

		}else if (count_NB_dif_A == NB_dif_A.size()){
			for(Vertex* a : A)
				a->data.resolved_status = VC;
			for(Vertex* b : B)
				b->data.resolved_status = EXCLUDED;
		}else{
			if(partial)
				return;
			assert(false && "this shouldn't be reached");
		}
    }

	void debug_print(Graph* G, bool partial){
		cout << "#### ALTERNATIVE (desk)\n";
		resolve(G, partial);
		cout << "#### DONE (hopefully not needed)\n";
	}
};
/* by Aleksander Figiel from "There and Back Again: On Applying Data Reduction Rules by Undoing Others",  	
 * https://doi.org/10.48550/arXiv.2206.14698
 * was unsure if our implementation from last year was buggy, but plan on replacing this again. Simply wanted to test if the rule actually helps or not.
 */
bool desk_rule_single(Graph &G, Vertex *a1) {
	bool success = false;
	if (deg(a1) != 3 && deg(a1) != 4)
		return false;
	vector<Vertex *> A;
	vector<Vertex *> B;
	// We try to construct A = {a1, a2}, and B = {b1, b2}
	// all vertices in N(A) are Amarked and all in N(B) are Bmarked

	vector<Vertex *> Bcand; // candidates for b1 and b2
	for (Vertex *u: a1->neighbors) {
		
		u->data.desk.Amarked = true;
		if (deg(u) == 3 || deg(u) == 4)
			Bcand.push_back(u);
	}
	if (Bcand.size() < 2)
		goto skipa1;

	for (size_t i = 0; i < Bcand.size(); i++) {
		Vertex *b1 = Bcand[i];

		size_t NB = 0; // |N(b1, b2)|
		for (Vertex *u: b1->neighbors) {
			if (u->data.desk.Amarked == true)
				goto skipb1;
			
			u->data.desk.Bmarked = true;
			NB++;
		}

		for (size_t j = i+1; j < Bcand.size(); j++) {
			Vertex *b2 = Bcand[j];
			if (b2->data.desk.Bmarked == true)
				continue;
			vector<Vertex *> Nb2; // N(b2) \ N(b1) \ a1
			vector<Vertex *> Ncap; // N(b1) cap N(b2)
			for (Vertex *u: b2->neighbors) {
				if (u->data.desk.Amarked == true)
					goto skipb2;
				if (u->data.desk.Bmarked == true) {
					Ncap.push_back(u);
					continue;
				}
				
				u->data.desk.Bmarked = true;
				Nb2.push_back(u);
			}
			NB += Nb2.size();

			if (NB > 4) // > 4 since a1,a2 are in N(b1) cap N(b2)
				goto skipb2;
			// for a2 in ((N(b1) cap N(b2)))
			for (Vertex *a2: Ncap) {
				if (a2 == a1 || a2->data.desk.Amarked == true)
					continue;
				if (deg(a2) != 3 && deg(a2) != 4)
					continue;

				size_t NA = deg(a1); // |N(a1, a2)|
				for (Vertex *u: a2->neighbors) {
					if (u->data.desk.Bmarked == true)
						goto skipa2;
					if (u->data.desk.Amarked == true)
						continue;
					NA++;
				}
				if (NA <= 4) { // 4 since b1,b2 are included
					success = true;
					A = {a1, a2};
					B = {b1, b2};
				}

			skipa2:
				if (success)
					break;
			}
		skipb2:
			NB -= Nb2.size();
			for (Vertex *u: Nb2)
				u->data.desk.Bmarked = false;
			if (success)
				break;
		}
	skipb1:
		for (Vertex *u: b1->neighbors) {
			u->data.desk.Bmarked = false;
		}
		if (success)
			break;
	}

skipa1:
	for (Vertex *u: a1->neighbors)
		u->data.desk.Amarked = false;

	if (success) {
		G.history.push_back(new VC_alternative(A, B, G));
	}
	return success;
}

// apply desk rule
bool desk_rule(Graph& G) {
	bool reduced = false;
    bool reduced_once = false;
	
	G.new_timestamp();

	for(size_t i = G.max_degree; i > 0; i--){
        for(Vertex* v : G.deg_lists[i]){
            v->data.desk.Amarked = false;
			v->data.desk.Bmarked = false;
		}
    }

	do {
		reduced = false;

		for(size_t i = 3; i <= 4; i++){
            size_t j = 0;
            while(G.max_degree >= i && j < G.deg_lists[i].size()){
                Vertex* v = G.deg_lists[i][j];
				bool was_unknown = v->status == UNKNOWN;
                if(desk_rule_single(G, v)) {
					assert(was_unknown);
                    reduced = true;
                    reduced_once   = true;
                    deg0_rule(G);
                }
                j++;
            }
        }
	}
	while (reduced);

	deg0_rule(G);

	return reduced_once;
}
