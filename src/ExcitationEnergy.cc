#include "ExcitationEnergy.hh"

#include "Ericson.hh"
#include "GaimardSchmidt.hh"

ExcitationEnergy::ExcitationEnergy(G4int ExEnLabel_in, G4int initA_in): ExEnLabel(3) {
ExEnLabel = ExEnLabel_in;
initA = initA_in;

G4cout<<"Excitation energy will be calculated with the "<<ExEnLabel_in<<" way"<<G4endl;

    LowExEn = 0*initA;
    UpExEn = 100*initA;
    Ebound = 40;

    ParamFile.open("../src/CorrectedALADINParameters.dat");
    SetParametersCorrectedALADINFromFile();
}

ExcitationEnergy::~ExcitationEnergy(){}

void ExcitationEnergy::SetInitNuclMass(G4int initA_in) {
    initA = initA_in;
}

void ExcitationEnergy::SetParametersALADIN(G4double e0_in, G4double sigma0_in, G4double b0_in) {
    e0 = e0_in;
    sigma0 = sigma0_in;
    b0 = b0_in;
 }

void ExcitationEnergy::SetParametersEricson(G4double g0_in) {
    g0 = g0_in;
}

void ExcitationEnergy::SetParametersGaimardSchmidt(G4double g0_in, G4double g1_in) {
    g0 = g0_in;
    g1 = g1_in;
}

void ExcitationEnergy::SetParametersCorrectedALADIN(G4double e0_in, G4double c0_in, G4double sigma0_in, G4double b0_in, G4double b1_in) {
    e0 = e0_in;
    c0 = c0_in;
    sigma0 = sigma0_in;
    b0 = b0_in;
    b1 = b1_in;
}

void ExcitationEnergy::SetParametersCorrectedALADINFromFile() {
    std::vector<G4double> ParamVect;
    G4double param = NULL;
    G4int iter=0;
    while(1){
        ParamFile>>param;
        ParamVect.push_back(param);
        if(!ParamFile.good()) break;
        iter++;
    }
    SetParametersCorrectedALADIN(ParamVect.at(0),ParamVect.at(1),ParamVect.at(2),ParamVect.at(3),ParamVect.at(4));
}

void ExcitationEnergy::SetParametersParabolicApproximation(G4double Pe_in, G4double Pm_in, G4double sigmaP_in, G4double bP0_in, G4double bP1_in) {
    Pe = Pe_in;
    Pm = Pm_in;
    sigmaP = sigmaP_in;
    bP0 = bP0_in;
    bP1 = bP1_in;
}


G4double ExcitationEnergy::GetEnergyALADIN(G4int A) {
    CLHEP::RandGauss randGauss(0,1);
    G4double energy = 0;
    G4double alpha = G4double(A) / G4double(initA);
    G4double sigmaE = randGauss.shoot() * sigma0 * (1 + b0 * (1 - alpha));
    energy = e0 * A * pow(1 - alpha, 0.5) + A * sigmaE;

    while (energy < 0 || energy != energy ) {
        sigmaE = randGauss.shoot() * sigma0 * (1 + b0 * (1 - alpha));
        energy = e0 * A * pow(1 - alpha, 0.5) + A * sigmaE;
    }
    return energy;
}

G4double ExcitationEnergy::GetEnergyCorrectedALADIN(G4int A) {

    CLHEP::RandGauss randGauss(0,1);
    G4double energy = 0;
    G4double alpha = G4double(A) / G4double(initA);
    G4double sigmaE = randGauss.shoot() * sigma0 * (1 + b0 * (1 - alpha)+b1 * (1-alpha) * (1-alpha));
    energy = A*std::sqrt(std::sqrt(e0*e0 + c0*(1-alpha)) - e0) + A * sigmaE;
    while (energy < 0 || energy != energy) {
        sigmaE = randGauss.shoot() * sigma0 * (1 + b0 * (1 - alpha)+b1 * (1-alpha) * (1-alpha));
        energy = A*std::sqrt(std::sqrt(e0*e0 - c0*(1-alpha)) - e0) + A * sigmaE;
    }
    return energy;
}

G4double ExcitationEnergy::GetEnergyEricson(G4int A) {
    G4double ExcitationEnergyDistribution[10000];
    G4double Ericson(G4double, G4double, G4int ,G4int);

    for (G4int n = 0; n < 10000; n++) {
        G4double sum =
                Ericson(G4double(n) * ((UpExEn - LowExEn) / G4double(10000)),
                        Ebound, A, initA) * (UpExEn - LowExEn) / G4double(10000);
        ExcitationEnergyDistribution[n] = sum;
    }
    CLHEP::RandGeneral randGeneral(ExcitationEnergyDistribution, 10000);

    G4double energy =
            randGeneral.shoot() * (UpExEn - LowExEn) + LowExEn;

    return energy;
}

G4double ExcitationEnergy::GetEnergyGaimardSchmidt(G4int A) {
    G4double ExcitationEnergyDistribution[10000];
    G4double GaimardSchmidt(G4double, G4double, G4int ,G4int);

    for (G4int n = 0; n < 10000; n++) {
        G4double sum =
                GaimardSchmidt(G4double(n) * ((UpExEn - LowExEn) / G4double(10000)),
                        Ebound, A, initA) * (UpExEn - LowExEn) / G4double(10000);
        ExcitationEnergyDistribution[n] = sum;
    }
    CLHEP::RandGeneral randGeneral(ExcitationEnergyDistribution, 10000);

    G4double energy =
            randGeneral.shoot() * (UpExEn - LowExEn) + LowExEn;

    return energy;
}


G4double ExcitationEnergy::GetEnergyParabolicApproximation(G4int A) {

    CLHEP::RandGauss randGauss(0,1);
    G4double energy;
    G4double alpha = G4double(A)/G4double(initA);
    G4double sigmaE = randGauss.shoot() * sigmaP * (1 + bP0 * (1 - alpha)+bP1 * (1-alpha) * (1-alpha));
    energy = Pe*G4double(A)*(1 - alpha)*(alpha + Pm)+sigmaE;

    return energy;
}



G4double ExcitationEnergy::GetEnergy(G4int A) {
   G4double energy;
    switch(ExEnLabel) {
        case 1:{
            energy = GetEnergyEricson(A);
            break;
        }
        case 2:{
            energy = GetEnergyGaimardSchmidt(A);
            break;
        }
        case 3:{
            energy = GetEnergyALADIN(A);
            break;
        }
        case 4:{
            energy = GetEnergyCorrectedALADIN(A);
            break;
        }
        case 5:{
            energy = GetEnergyParabolicApproximation(A);
            break;
        }
        default:{
            G4Exception("Statistics label", "GRATE-1", FatalException, "Statistics label is invalid");
            break;
        }
    }
    return energy;
}

