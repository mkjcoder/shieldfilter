
void adjustEdgesIfNecessary(nodeP* np){
    if(np->edges == NULL || np->nIdx >= np->maxEdges){
    np->maxEdges += 5;
    edgeP* newep = (edgeP*) calloc(np->maxEdges,sizeof(edgeP));
    if(np->edges != NULL) { 
      memcpy(newep,np->edges,np->nIdx*sizeof(edgeP));
      for(int i=0; i<np->nIdx; i++){
        newep[i].rev->rev = newep+i;
      }
      free(np->edges); 
    }
    np->edges = newep;
  }
}


/* ----------------------------------------------------------------- */

int parse( n_ad, m_ad, nodes_ad)

/* all parameters are output */
long    *n_ad;                 /* address of the number of nodes */
long    *m_ad;                 /* address of the number of arcs */
nodeP    **nodes_ad;            /* address of the array of nodes */

{

#define MAXLINE       100000000	/* max line length in the input file */
#define ARC_FIELDS      3	/* no of fields in arc line  */
#define NODE_FIELDS     2	/* no of fields in node line  */
#define P_FIELDS        3       /* no of fields in problem line */
#define PROBLEM_TYPE "max"      /* name of problem type*/































long    no_lines=0,             /* no of current input line */
        no_plines=0,            /* no of problem-lines */
        // no_nslines=0,           /* no of node-source-lines */
        // no_nklines=0,           /* no of node-source-lines */
        no_alines=0;            /* no of arc-lines */
        // pos_current=0;          /* 2*no_alines */

char*    in_line = (char*) calloc ( MAXLINE, 1);    /* for reading input line */
char     pr_type[3];             /* for reading type of the problem */
        // nd;                     /* source (s) or sink (t) */

int     err_no;                 /* no of detected error */

/* -------------- error numbers & error messages ---------------- */
#define EN1   0
#define EN2   1
#define EN3   2
#define EN4   3
#define EN6   4
#define EN10  5
#define EN7   6
#define EN8   7
#define EN9   8
#define EN11  9
#define EN12 10
#define EN13 11
#define EN14 12
#define EN16 13
#define EN15 14
#define EN17 15
#define EN18 16
#define EN21 17
#define EN19 18
#define EN20 19
#define EN22 20

static char *err_message[] = 
  { 
/* 0*/    "more than one problem line.",
/* 1*/    "wrong number of parameters in the problem line.",
/* 2*/    "it is not a Max Flow problem line.",
/* 3*/    "bad value of a parameter in the problem line.",
/* 4*/    "can't obtain enough memory to solve this problem.",
/* 5*/    "more than one line with the problem name.",
/* 6*/    "can't read problem name.",
/* 7*/    "problem description must be before  description.",
/* 8*/    "this parser doesn't support multiply sources and sinks.",
/* 9*/    "wrong number of parameters in the node line.",
/*10*/    "wrong value of parameters in the node line.",
/*11*/    " ",
/*12*/    "source and sink descriptions must be before arc descriptions.",
/*13*/    "too many arcs in the input.",
/*14*/    "wrong number of parameters in the arc line.",
/*15*/    "wrong value of parameters in the arc line.",
/*16*/    "unknown line type in the input.",
/*17*/    "reading error.",
/*18*/    "not enough arcs in the input.",
/*19*/    "source or sink doesn't have incident arcs.",
/*20*/    "can't read anything from the input file."
  };
/* --------------------------------------------------------------- */

/* The main loop:
        -  reads the line of the input,
        -  analises its type,
        -  checks correctness of parameters,
        -  puts data to the arrays,
        -  does service functions
*/
long n,m,head,tail,cap;//source,sink,
while (fgets(in_line, MAXLINE, stdin) != NULL )
  {
  no_lines ++;
  if(no_lines % 10000 == 0){
    printf("c line %ld\n",no_lines);
  }

  switch (in_line[0])
    {
      case 'c':                  /* skip lines with comments */
      case '\n':                 /* skip empty lines   */
      case '\0':                 /* skip empty lines at the end of file */
                break;

      case 'p':                  /* problem description      */
                if ( no_plines > 0 )
                   /* more than one problem line */
                   { err_no = EN1 ; goto error; }

                no_plines = 1;
   
                if (
        /* reading problem line: type of problem, no of nodes, no of arcs */
                    sscanf ( in_line, "%*c %3s %ld %ld", pr_type, &n, &m )
                != P_FIELDS
                   )
		    /*wrong number of parameters in the problem line*/
		    { err_no = EN2; goto error; }

                if ( strcmp ( pr_type, PROBLEM_TYPE ) )
		    /*wrong problem type*/
		    { err_no = EN3; goto error; }

                if ( n <= 0  || m <= 0 )
		    /*wrong value of no of arcs or nodes*/
		    { err_no = EN4; goto error; }

        /* allocating memory for  'nodes', 'arcs'  and internal arrays */
                *n_ad = n;
                *m_ad = m;
                *nodes_ad =  (nodeP*) calloc ( n+2, sizeof(nodeP) );
                memset(*nodes_ad,0,(n+2)*sizeof(nodeP));

                break;

      case 'n':		         /* source(s) description */
		// if ( no_plines == 0 )
    //               /* there was not problem line above */
    //               { err_no = EN8; goto error; }

    //             /* reading source  or sink */
		// k = sscanf ( in_line,"%*c %ld %c", &i, &nd );
 
		// if ( k < NODE_FIELDS )
    //               /* node line is incorrect */
    //               { err_no = EN11; goto error; }

		// if ( i < 0 || i > n )
    //               /* wrong value of node */
    //               { err_no = EN12; goto error; }

		// switch ( nd )
		//   {
		//   case 's':  /* source line */
		    
		//     if ( no_nslines != 0)
		//       /* more than one source line */ 
		//       { err_no = EN9; goto error; }

		//     no_nslines = 1;
		//     source = i;
		//     break;

		//   case 't':  /* sink line */

		//     if ( no_nklines != 0)
		//       /* more than one sink line */
		//       { err_no = EN9; goto error; }

		//     no_nklines = 1;
		//     sink = i;
		//     break;

		//   default:
		//     /* wrong type of node-line */
    //                 err_no = EN12; goto error; 
		//     break;
		//   }


		break;

      case 'a':                    /* arc description */
		// if ( no_nslines == 0 || no_nklines == 0 ) 
    //               /* there was not source and sink description above */
    //               { err_no = EN14; goto error; }

		if ( no_alines >= m )
                  /*too many arcs on input*/
                  { err_no = EN16; goto error; }
		
		if (
                    /* reading an arc description */
                    sscanf ( in_line,"%*c %ld %ld %ld",
                                      &head, &tail, &cap )
                    != ARC_FIELDS
                   ) 
                    /* arc description is not correct */
                    { err_no = EN15; goto error; }

		if ( tail < 0  ||  tail > n  ||
                     head < 0  ||  head > n  
		   )
                    /* wrong value of nodes */
		    { err_no = EN17; goto error; }

               /* no of arcs incident to node i is stored in arc_first[i+1] */
	/*add node prop*/
  // printf("read %ld %ld %ld\n",head,tail,cap);
  no_alines ++;
  nodeP* np = *nodes_ad+head;
  adjustEdgesIfNecessary(np);

  edgeP* newedge = np->edges+np->nIdx;
  newedge->endNode = tail;
  newedge->w = 1;
  newedge->cap = cap;
  np->nIdx ++;

  edgeP* newedge_prev = newedge;
  
  np = *nodes_ad + tail;
  adjustEdgesIfNecessary(np);

  newedge = np->edges + np->nIdx;
  newedge->endNode = head;
  newedge->w = 1;
  newedge->cap = cap;
  np->nIdx ++;
  
  newedge_prev->rev = newedge;
  newedge->rev = newedge_prev;

		break;

	default:
		/* unknown type of line */
		err_no = EN18; goto error;
		break;

    } /* end of switch */
}     /* end of input loop */

/* ----- all is red  or  error while reading ----- */ 

if ( feof (stdin) == 0 ) /* reading error */
  { err_no=EN21; goto error; } 

if ( no_lines == 0 ) /* empty input */
  { err_no = EN22; goto error; } 

if ( no_alines < m ) /* not enough arcs */
  { err_no = EN19; goto error; } 











 




























	    








































































  









return (0);

/* ---------------------------------- */
 error:  /* error found reading input */

printf ( "\nline %ld of input - %s\n", 
         no_lines, err_message[err_no] );

exit (1);

}
/* --------------------   end of parser  -------------------*/
