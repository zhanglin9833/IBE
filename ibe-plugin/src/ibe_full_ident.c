/*
 Boneh-Franklin Identity-Based Encryption from the Weil Pairing
 
 Flow Chart:
 (1)Setup:Take secruity parameter K(QBITS,RBITS),return the system parameter and master
 key of the PKG.The system parameters include a description of a finite message space M,
 and a dscription of a finite ciphertext space C. The system parameters will be publicly
 known,while the master-key will be known only to the PKG.
 (2)Extract:The receiver extracts the corresponding private key from the PKG.
 (3)Encrypt:The sender will generate a ciphertext based on the receiver ID.
 (4)Decrypt:The receiver will use his private key to get the message digest.
 
 Detail:
 1.H1 function---Element build-in function(element_from_hash)
 2.H2 function---SHA1 function generate 160 bit long number
 3.H3 function---Concatenate the sigma and message digest, and
 then put it into build-in function element_random. The random number will between 0 and q.
 4.H4 function---Input a 160 bit long number and run SHA1 function to generate another 160 bit long number.
 5.As use SHA1 function as H2 function, thus the n is automatically set as 160.
 */


#include "ibe_full_ident.h"

#define SIZE 1000
#define RBITS 160
#define QBITS 512
#define ELE_BASE 10 
#define PATH_PBC_PARAM "/home/dell/github/GraduationProject/ibe-plugin/src/ibe_parameters/pbc_param.txt"
#define PATH_P "/home/dell/github/GraduationProject/ibe-plugin/src/ibe_parameters/P.txt"
#define PATH_Ppub "/home/dell/github/GraduationProject/ibe-plugin/src/ibe_parameters/Ppub.txt"
#define PATH_s "/home/dell/github/GraduationProject/ibe-plugin/src/ibe_parameters/s.txt"
#define PATH_Sid "/home/dell/github/GraduationProject/ibe-plugin/src/ibe_parameters/Sid.txt"


void get_private_key(element_t Sid)
{
  FILE *fp;
  char tmp[SIZE] = {'\0'};
  fp = fopen(PATH_Sid, "r");
  fgets(tmp, SIZE, fp);
  element_set_str(Sid, tmp, ELE_BASE); 
  fclose(fp);
  fp = NULL;
  element_printf("Private key Sid = %B\n", Sid);
}

void get_public_key(char* ID, element_t Qid)
{
  element_from_hash(Qid, ID, strlen(ID));
  element_printf("\nPublic key Qid = %B\n", Qid);
}

void setup_sys(element_t P, element_t Ppub, pairing_t pairing, element_t s)
{
  pbc_param_t par;   //Parameter to generate the pairing
  char params[SIZE] = {'\0'};
  FILE *pbc_param_file = fopen(PATH_PBC_PARAM, "r");
  if (pbc_param_file == NULL)
      printf("\n******************\n");
  fread(params, 1, SIZE, pbc_param_file);
  fclose(pbc_param_file);
  pbc_param_file = NULL;
  pbc_param_init_set_str(par, params);
  pairing_init_pbc_param(pairing, par); //Initial the pairing

  printf("\ninit pbc param finished\n");

  //In our case, the pairing must be symmetric
  if (!pairing_is_symmetric(pairing))
  pbc_die("pairing must be symmetric");
  
  element_init_G1(P, pairing);
  element_init_G1(Ppub, pairing);
  element_init_Zr(s, pairing);
  FILE *fp;
  char tmp[SIZE] = {'\0'};
  fp = fopen(PATH_P, "r");  
  fgets(tmp, SIZE, fp);
  printf("\n^^%s\n", tmp);
  element_set_str(P, tmp, ELE_BASE);
  fclose(fp);
  /*printf("\ncnt = %d\n", cnt);*/

  fp = fopen(PATH_Ppub, "r");
  fgets(tmp, SIZE, fp);
  element_set_str(Ppub, tmp, ELE_BASE);
  fclose(fp);

  fp = fopen(PATH_s, "r");
  fgets(tmp, SIZE, fp);
  element_set_str(s, tmp, ELE_BASE);
  fclose(fp);
  fp = NULL;
}

void rand_n(char* sigma)
{
  int i;
  int unit;
  char tempr[10];
  memset(sigma, 0, sizeof(char)*SIZE);//Clear the memory of sigma
  
  for (i = 0; i < 40; i++)
  {
  unit = rand() % 16;
  sprintf(tempr, "%X", unit);
  strcat(sigma, tempr);
  }
}

int* get_value(char *V)
{

    int i, j, tmp_value, V_len;
    int value[40];
    memset(value, 0, sizeof(value));
    V_len = strlen(V);
    /*printf("\n\nV_len = %d\n", V_len);*/
    j = 0;
    for (i = 0; i < V_len;)
    {
        tmp_value = 0;
        if (V[i] != ' ')
        {
            tmp_value += 16 * htoi(V[i]);
        }
        ++i;
        tmp_value += htoi(V[i]);
        /*printf("%d ", tmp_value);*/
        value[j++] = tmp_value;
        ++i;
    }
    /*printf("\n\n");*/
    return value;    
}

void decryption(element_t Sid,pairing_t pairing,element_t P,element_t U,char* V,char* W,element_t U_receiver,char* tmp_msg)
{
  
  int i;
  element_t rgid;
  element_t r_receiver;
  char sgid_receiver[SIZE]; //Receiver calculated gid string representation
  char shagid_receiver[SIZE]; //Receiver H2 function result
  char sigma_receiver[SIZE]; //Receiver compute the sigma
  char ssigma_receiver[SIZE]; //It is the result of H4(sigma_receiver)
  char msigma_receiver[2*SIZE]; //Receiver concatenate the sigma and message digest
  memset(sigma_receiver, 0, sizeof(char)*SIZE);//Clear the memory of sigma_receiver
  /*memset(shamessage_receiver, 0, sizeof(char)*SIZE);//Clear the memory of shamessage_receiver*/
  
  element_init_Zr(r_receiver,pairing);
  element_init_GT(rgid, pairing);
  element_pairing(rgid, Sid, U);
  element_printf("\nrgid = %B\n", rgid);
  element_snprint(sgid_receiver, SIZE, rgid);
  sha_fun(sgid_receiver, shagid_receiver); //Generate H2(e(dID,U));

  
  /*printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");*/

  int *V_val = get_value(V);
  //XOR V and H2(e(dID,U))=sigma_receiver
  for (i = 0; i < 40; ++i)
  {
      sigma_receiver[i] = V_val[i] ^ (int)shagid_receiver[i];
  /*xor_operation(V[i], shagid_receiver[i], sigma_receiver);*/
  }
  
  sha_fun(sigma_receiver, ssigma_receiver);

  printf("\n\nG1(sigma_receiver) = %s\n", ssigma_receiver);
  
  int *W_val = get_value(W);
  //XOR W andH4(sigma)
  for (i = 0; i < 40; ++i)
  {
      tmp_msg[i] = W_val[i] ^ (int)ssigma_receiver[i];

  /*xor_operation(W[i], ssigma_receiver[i], shamessage_receiver);*/
  }
  
  strcpy(msigma_receiver, sigma_receiver);
  /*strcat(msigma_receiver, shamessage_receiver);*/
  strcat(msigma_receiver, tmp_msg);
  element_from_hash(r_receiver, msigma_receiver, strlen(msigma_receiver));
  element_mul_zn(U_receiver, P, r_receiver);
}

char* decrypt_mail_msg(char *encrypted_mail_msg)
{

    printf("\n##########DECRYPTION##########\n");
    char decrypted_mail_msg[SIZE] = {'\0'};
    int encrypted_mail_msg_len = strlen(encrypted_mail_msg);
    element_t P, Ppub, s, U_receiver, U, Sid;
    pairing_t pairing;
    int i, j, d, k;
    setup_sys(P, Ppub, pairing, s);
    element_printf("\nP = %B\n", P);
    element_printf("\nPpub = %B\n", Ppub);
    element_printf("\n**s = %B\n", s);

    printf("\ninit ibe system parameters finished\n");
    element_init_G1(Sid, pairing);
    get_private_key(Sid);

    char tmp_U[10*SIZE];
    char V[2*SIZE];
    char W[2*SIZE];
    char tmp_msg[45];
    for (i = 0; i < encrypted_mail_msg_len; ++i)
    {
        memset(tmp_U, 0, sizeof(tmp_U));
        j = 0;        
        while (i < encrypted_mail_msg_len && encrypted_mail_msg[i] != '&')
        {
            tmp_U[j++] = encrypted_mail_msg[i++];
        }    
        /*printf("\ntmp_U = %s\n", tmp_U);*/
        element_init_G1(U, pairing);
        element_set_str(U, tmp_U, ELE_BASE);
        element_printf("\nU = %B\n", U);
        
        ++i;
        memset(V, 0, sizeof(V));
        j = 0;
        while (i < encrypted_mail_msg_len && encrypted_mail_msg[i] != '&')
        {
            V[j++] = encrypted_mail_msg[i++];
        }
        /*printf("\nV = %s\n", V);*/
        
        ++i;
        memset(W, 0, sizeof(W));
        j = 0;
        while (i < encrypted_mail_msg_len && encrypted_mail_msg[i] != '&')
        {
            W[j++] = encrypted_mail_msg[i++];
        }
        /*printf("\nW = %s\n", W);*/
 
        element_init_G1(U_receiver, pairing);
        memset(tmp_msg, 0, sizeof(tmp_msg)); 
        decryption(Sid, pairing, P, U, V, W, U_receiver, tmp_msg);
        /*printf("\n\ntmp_msg is: %s\n", tmp_msg);
        printf("\n\ntmp_msg is: \n");
        for (d = 0; d < 40; ++d)
            printf("%d ", (int)tmp_msg[d]);*/
        if (element_cmp(U, U_receiver) == 0)
        {
            element_printf("\nU=%B", U);
            element_printf("\nU_receiver=%B", U_receiver);
            d = 0;
            while ((int)tmp_msg[d] == 1)
            {
                ++d;
            }
            /*printf("\n\n$$$$$$$d = %d\n", d);*/
            if (d == 0)
            {
                strcat(decrypted_mail_msg, tmp_msg);
            }
            else 
            {
                char tmp_msg_delhead[40] = {'\0'};
                for (k = 0; d < 40; ++d, ++k)
                    tmp_msg_delhead[k] = tmp_msg[d];
                strcat(decrypted_mail_msg, tmp_msg_delhead);
            }
        }

        else
        {
            /*element_printf("\nU=%B", U);
              element_printf("\nU_receiver=%B", U_receiver);*/
            printf("\nOops!The ciphertext can not be accepted!\n");
            return NULL;
        }
    } 

    printf("\nYeah!The message is decrpted!");
    printf("\nThe decrypted Message =\n%s\n\n", decrypted_mail_msg);
    /*int ll;
    int len_mail = strlen(decrypted_mail_msg);
    for (ll = 0; ll < len_mail; ++ll)
        printf("%d ", decrypted_mail_msg[ll]);
    printf("\n\n\n");*/
    return decrypted_mail_msg;
}

void encryption(char* shamessage, char* ID, element_t P, element_t Ppub, element_t U, char *V, char *W, pairing_t pairing)
{
  int i;
  char sgid[SIZE];   //Sender gid string representation
  char shagid[SIZE]; //Sender H2 function result
  char sigma[SIZE]; //Sender generate the sigma
  char msigma[2*SIZE]; //Sender concatenate the sigma and message digest
  char ssigma[SIZE]; //It is the result of H4(sigma)
  element_t r;
  element_t Qid;
  element_t gid;
  element_init_G1(Qid, pairing);
  element_init_GT(gid, pairing);
  element_init_Zr(r, pairing);
  rand_n(sigma); //Sender generate a sigma
  strcpy(msigma, sigma);
  strcat(msigma, shamessage);
  element_from_hash(r, msigma, strlen(msigma));
  element_mul_zn(U, P, r);
  element_printf("\nr = %B", r);
  element_printf("\nU = %B", U);
  get_public_key(ID, Qid);
  element_pairing(gid, Qid, Ppub);
  element_pow_zn(gid, gid, r);
  element_printf("\ngid = %B\n", gid);
  element_snprint(sgid, SIZE, gid);
  sha_fun(sgid, shagid); //H2(gid^r)
  sha_fun(sigma, ssigma); //H4(SIGMA)

  printf("\n\nH(gidr) = %s\n", shagid);

  printf("\n\nG1(sigma) = %s\n", ssigma);
  
  printf("\nV = \n");
  int tmp_value = 0;
  char tmp_s[5];
  //Do the XOR operation to the sigma and shagid digest
  for (i = 0; i < 40; i++)
  {
      /*V[i] = (char)((int)sigma[i] ^ (int)shagid[i]);*/
      tmp_value = (int)sigma[i] ^ (int)shagid[i];
      memset(tmp_s, 0, sizeof(tmp_s));
      sprintf(tmp_s, "%2X", tmp_value);
      strcat(V, tmp_s);
      printf("%d, %d, %d, %s, %d, %s\n", (int)sigma[i], (int)shagid[i], tmp_value, tmp_s, tmp_s[0], V);
  /*xor_operation(sigma[i], shagid[i], V);*/
  }

  printf("\nW = \n");
  //Do the XOR operation to the ssigma and message digest
  for (i = 0; i < 40; i++)
  {
      /*W[i] = (char)((int)shamessage[i] ^ (int)shagid[i]);*/
      tmp_value = (int)shamessage[i] ^ (int)ssigma[i];
      memset(tmp_s, 0, sizeof(tmp_s));
      sprintf(tmp_s, "%2X", tmp_value);
      strcat(W, tmp_s);
      printf("%d, %d, %d, %s, %d, %s\n", (int)shamessage[i], (int)shagid[i], tmp_value, tmp_s, tmp_s[0], W);
  /*xor_operation(shamessage[i], ssigma[i], W);*/
  }
  
  printf("\nV=%s", V);
  printf("\nW=%s\n", W);
}

char* encrypt_mail_msg(char *mail_msg, char *ID)
{
  char encrypted_mail_msg[100*SIZE] = {'\0'};
  
  pairing_t pairing;   //The pair of bilinear map
  element_t P, Ppub, s, U, Qid;
  
  printf("\nID = %s\n", ID);
  printf("\nmail_msg:\n%s\n", mail_msg);
  
  printf("\n############SETUP############\n");

  setup_sys(P, Ppub, pairing, s);
  printf("System parameters have been set!\n");
  element_printf("P = %B\n", P);
  element_printf("Ppub = %B\n", Ppub);

  printf("###########EXTRACT###########\n");
  
  element_init_G1(Qid, pairing);
  get_public_key(ID, Qid);
  
  printf("##########ENCRPTION##########\n");
  
  printf("\nThe original message = %s\n", mail_msg);//
  int mail_msg_len = strlen(mail_msg);
  /*the encryption function dealt with 40 characters everytime*/
  int cnt = mail_msg_len / 40; 
  int re = mail_msg_len % 40;
  printf("\ncnt = %d, re = %d\n\n", cnt, re);
  char tmp_msg[40];
  char V[2*SIZE];
  char W[2*SIZE];
  char tmp_U[10*SIZE];
  memset(encrypted_mail_msg, 0, sizeof(encrypted_mail_msg));
  int i, j, k;
  for (i = 0; i < cnt; ++i)
  {
    element_init_G1(U, pairing);
    memset(tmp_msg, 0, sizeof(tmp_msg));
    memset(V, 0, sizeof(V));
    memset(W, 0, sizeof(W));
    memset(tmp_U, 0, sizeof(tmp_U));

    for (j = i*40, k = 0; j < (i+1)*40; ++j, ++k)
    {
        tmp_msg[k] = mail_msg[j];
    }
    printf("\ntmp_msg is: %s\n", tmp_msg);
    encryption(tmp_msg, ID, P, Ppub, U, V, W, pairing);
    element_snprint(tmp_U, SIZE, U);
    printf("\ntmp_U = %s\n", tmp_U);
    printf("\n\ntmp_U = %s\n", tmp_U);
    printf("\nV = %s\n", V);
    printf("\nW = %s\n", W);
    strcat(encrypted_mail_msg, tmp_U);
    strcat(encrypted_mail_msg, "&");
    strcat(encrypted_mail_msg, V);
    strcat(encrypted_mail_msg, "&");
    strcat(encrypted_mail_msg, W);
    strcat(encrypted_mail_msg, "&"); 
  }  
  if (re != 0)
  {
    element_init_G1(U, pairing);
    memset(tmp_msg, 0, sizeof(tmp_msg));
    memset(V, 0, sizeof(V));
    memset(W, 0, sizeof(W));
    memset(tmp_U, 0, sizeof(tmp_U));

    printf("\n***************tmp_msg is:**********\n");
    for (k = 0; k < 40 - re; ++k)
    {
        tmp_msg[k] = 1;
        printf("%d ", tmp_msg[k]);

    }
    for (j = 40 * cnt, k = 40 - re; j < mail_msg_len; ++j, ++k)
    {
        tmp_msg[k] = mail_msg[j];
        printf("%d ", tmp_msg[k]);
    }
    printf("\n\n");
 
    encryption(tmp_msg, ID, P, Ppub, U, V, W, pairing);
    element_snprint(tmp_U, SIZE, U);
    printf("\n\ntmp_U = %s\n", tmp_U);
    printf("\nV = %s\n", V);
    printf("\nW = %s\n", W);
    strcat(encrypted_mail_msg, tmp_U);
    strcat(encrypted_mail_msg, "&");
    strcat(encrypted_mail_msg, V);
    strcat(encrypted_mail_msg, "&");
    strcat(encrypted_mail_msg, W);
    strcat(encrypted_mail_msg, "&"); 
    
  }
      
  printf("Send (U,V,W) to the receiver!");
  element_clear(P);
  element_clear(Ppub);
  element_clear(Qid);
  element_clear(U);
  element_clear(s);
  pairing_clear(pairing);
  
  printf("\nencrypted_mail_msg:\n%s\n", encrypted_mail_msg);
  return encrypted_mail_msg;
}


