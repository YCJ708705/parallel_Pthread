
#include "database.h"
#include <string.h>
//#include <pthread.h>
#include <string>
#include <vector>
#include <time.h>
#include <list>
#include <math.h>
#include <thread>
#define PROFILE_MODE 1

int thread_use;
void parallel_read(int id_thread);

class target{
private:
    double theta;
    double phi;
    double rd;
    double Mag; 
public:
    double get_theta(){return theta;}
    double get_phi(){return phi;}
    double get_r(){return rd;}
    double get_mag(){return Mag;}
    void set_position(double *Peak){
        theta=Peak[0];
        phi=Peak[1];
        rd=Peak[2];
        Mag=Peak[3];
    }
};

target decoder_index(double *PeakData){
    target object_new;
    //double r_meters = ((Rdistance_unit * Urd)-(PeakData[2]))*MetersPerPoint;
    
    double theta_degree=(((PeakData[0]+1)-(NTx/2)-1)*ThPerPoint);
    double phi_degree=(((PeakData[1]+1)-(NRx/2)-1)*PhPerPoint);
    double r_meters=(-(PeakData[2]+1)+(NRd/2)+1)*MetersPerPoint;
    
    double *polar_data=new double[4];
    polar_data[0]=theta_degree;
    polar_data[1]=phi_degree;
    polar_data[2]=r_meters;
    polar_data[3]=PeakData[3];
    //set object's position information
    object_new.set_position(polar_data);
    free(polar_data);
    return object_new;
}
void write_list(std::list<target> target_list){
    int lenlist=target_list.size();
    FILE *fo = fopen("result_list.txt","w");
    
    for (list <target>::iterator iTar = target_list.begin(); iTar != target_list.end(); iTar++){
        fprintf(fo,"(%f,%f,%f) => %f \n",iTar->get_theta(),iTar->get_phi(),iTar->get_r(),iTar->get_mag());
    }
    
    fclose(fo);
}


database Data(NTx,NRx,NRd,DataTypeNum);



int main (int argc, char* argv[]){
    
    //give num_of_threads
    thread_use =  strtol(argv[1], NULL, 10);
    
    
    std::list<target> target_list;
    clock_t read_start, read_end;
    
    //create thread
    thread thread_pool[thread_use];
    read_start = clock();
    for(long ithread=0;ithread<thread_use;ithread++){
        thread_pool[ithread] = std::thread(parallel_read,ithread);
    }
    for(int ithread=0;ithread<thread_use;ithread++){
        thread_pool[ithread].join();
    }
    read_end= clock();
    
    
    
    clock_t find_start, find_end;
    find_start = clock();
    for(int iTar=0;iTar<10;iTar++){
        double *Peak=Data.FindMaxPeak();
        target object_new = decoder_index(Peak);
        target_list.push_front(object_new);
    }
    find_end = clock();
    write_list(target_list);
    double read_time_cost=((double) (read_end - read_start)) / CLOCKS_PER_SEC;
    cout<<"read_time : "<<read_time_cost<<"sec."<<endl;
    double search_time_cost=((double) (find_end - find_start)) / CLOCKS_PER_SEC;
    cout<<"search_time : "<<search_time_cost<<"sec."<<endl;
    //file.open(“Reader.txt”,ios::in) ;    
    Data.writedebug();
    //string mLine = mFileLineGet.readLine();
    
}//end main



////paral_function

void parallel_read(int id_thread){
    
    string pre_input_file;
    //pre_input_file = "D:/google_drive/finalproject";
    pre_input_file = "./file";
    //pre_input_file = "/home/PP-s19/pp10767219/project";
    string MidSamePart="/DSP_output_16ch_TxAngle_";   
    // long  *iworker = new long[1];
    int work_beg = id_thread;
    
    
    for(int iTx=work_beg;iTx<NTx;iTx=iTx+thread_use){
        
        string inum =to_string(iTx);
        string wholefilepath=pre_input_file+MidSamePart+inum+"_repeat_8.dat";
        int len_path = wholefilepath.length(); 
        char char_array[100000]; 
        memset(char_array, '\0', sizeof(char_array));
        strcpy(char_array, wholefilepath.c_str()); 
        ifstream myfile ;
        myfile.open( char_array , ifstream::in ); 
        if(!myfile){
            cout<<"readfile fail!!"<<endl;
            exit;
        }
        int numLine=0;
        for(int iRx=0 ; iRx<NRx ; iRx++){
            for(int iRd=0; iRd<NRd ; iRd++){
                string mLine;
                getline(myfile,mLine);
                int num_mLine=mLine.length();
                int Np0=mLine.find(" ",2);
                
                char MagMidRe[17]; 
                memset(MagMidRe, '\0', sizeof(MagMidRe));
                char MagMidIm[17]; 
                memset(MagMidIm, '\0', sizeof(MagMidIm));
                mLine.copy(MagMidRe,Np0,0);
                int num_remain=num_mLine-Np0;
                mLine.copy(MagMidIm,num_remain,Np0);
                
                double Magnitude_RealPart=atof(MagMidRe);
                double Magnitude_ImagPart=atof(MagMidIm);
                Data.setData(Magnitude_RealPart,iTx,iRx,iRd,0);
                Data.setData(Magnitude_ImagPart,iTx,iRx,iRd,1);
                double absMag=sqrt(Magnitude_RealPart*Magnitude_RealPart+Magnitude_ImagPart*Magnitude_ImagPart);
                Data.setData(absMag,iTx,iRx,iRd,2);
                numLine++;
            }
        }
        
        //cout<<"File "<<iTx<<endl;
        //cout<<"File "<<iTx<<" :"<<numLine<<endl;
        myfile.close();
        
    }
    
}
