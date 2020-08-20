#include <string>		// string, to_string
#include <vector>		// vector
#include <iostream>		// cout

using namespace std;

class ModeDim
{
protected:
	int curr, max, type, settings;
	bool locked;
	vector<string> modeNames, settingNames;
	vector<int> locks;

	vector<string> expandFromName(string name, int sets)
	{
		vector<string> res;

		for (int i = 0; i < max; ++i)
			res.push_back(name + to_string(i));

		return res;
	}

	vector<string> genModeNames(string names[], int sets)
	{
		vector<string> res;

		for (int i = 0; i < max; ++i)
		{
			string name = "";
			for (int j = 0; j < sets; ++j)
				if ((i >> j) % 2 == 1)
					name += names[j + 1];

			if (name == "")
				name = names[0];

			res.push_back(name + names[sets + 1]);
		}

		return res;
	}

	vector<string> genSettingNames(string names[], int sets)
	{
		vector<string> res;

		for (int i = 0; i < sets; ++i)
		{
			res.push_back("N" + names[i + 1] + names[sets + 1]);
			res.push_back("Y" + names[i + 1] + names[sets + 1]);
		}

		return res;
	}

public:
	ModeDim(string name, int type = 0, int max = 2)
	{
		this->curr = 0;
		if (type == 0)
		{
			this->settings = max;
			this->max = max;
			this->locks = vector<int>(1, -1);
		}
		else if (type == 1)
		{
			this->settings = 2 * max;
			this->max = pow(2, max);
			this->locks = vector<int>(max, -1);
		}
		this->type = type;
		this->locked = false;
		this->modeNames = expandFromName(name, max);
		this->settingNames = modeNames;
	}

	ModeDim(string names[], int type = 0, int max = 2)
	{
		this->curr = 0;
		this->type = type;
		this->locked = false;
		if (type == 0)
		{
			this->settings = max;
			this->max = max;
			this->modeNames = vector<string>(names, names + max);
			this->settingNames = vector<string>(names, names + max);
			this->locks = vector<int>(1, -1);
		}
		else if (type == 1)
		{
			this->settings = 2 * max;
			this->max = pow(2, max);
			this->modeNames = genModeNames(names, max);
			this->settingNames = genSettingNames(names, max);
			this->locks = vector<int>(max, -1);
		}
	}

	int next()
	{
		if (locked)
			return -1;

		do
			++curr;
		while (!checkLocks());

		if (curr < max)
			return curr;
		else
		{
			curr = 0;
			if (!checkLocks())
				curr = next();
			return -1;
		}
	}

	bool checkLocks()
	{
		for (int i = 0; i < locks.size(); ++i)
			if (locks[i] != -1 && getCurr(i) != locks[i])
				return false;
		return true;
	}

	int getCurr(int dim = -1)
	{
		if (type == 0)
			return curr;
		else if (type == 1 && dim >= 0)
			return (curr >> dim) % 2;
	}

	int getCurr(string name)
	{
		if (type == 1)
			name = "Y" + name;

		for (int i = 0; i < settings; ++i)
			if (name.compare(settingNames[i]) == 0)
				return getCurrBySet(i);

		return -1;
	}

	bool getCurrBySet(int set)
	{
		if (type == 0)
			return curr == set;
		else if (type == 1)
			return getCurr(set / 2) == set % 2;
	}

	vector<bool> getSetStat()
	{
		vector<bool> res;

		for (int i = 0; i < settings; ++i)
			res.push_back(getCurrBySet(i));

		return res;
	}

	int getSetID(string name)
	{
		if (type == 1)
			name = "Y" + name;

		int res;

		for (int i = 0; i < settingNames.size(); ++i)
			if (settingNames[i].compare(name) == 0)
			{
				res = i;
				if (type == 1)
					res /= 2;
				return res;
			}

		return -1;
	}

	void setCurr(int val)
	{
		if (!locked)
			curr = val;
	}

	void lock(int val)
	{
		curr = val;
		locked = true;
	}

	void lockSetting(int set, int val)
	{
		locks[set] = val;
	}

	string getModeName(int mode = -1)
	{
		int id = mode;
		if (id == -1)
			id = curr;

		return modeNames[id];
	}

	string getSetName(int set = -1)
	{
		int id = set;
		if (id == -1)
			id = curr;

		return settingNames[id];
	}

	vector<string> getSubModeNames()
	{
		if (type == 0)
			return vector<string>();
		else if (type == 1)
			return modeNames;
	}

	int getMax()
	{
		return this->max;
	}

	int getSettings()
	{
		return this->settings;
	}
};

class Mode
{
private:
	int nDims, nModes, nSettings, current;
	vector<ModeDim> dims;
	vector<double> durs;
	bool locked;

	// Updates the int members after a dimension is added
	void updateCounters()
	{
		int newModes = dims[nDims].getMax();

		if (nModes == 0)
			nModes += newModes;
		else
			nModes *= newModes;
		nSettings += dims[nDims].getSettings();
		nDims++;
	}

public:
	// Constructor
	Mode()
	{
		nDims = 0;
		nModes = 0;
		nSettings = 0;
		current = 0;
		durs = vector<double>();
		locked = false;
	}

	void Resize()
	{
		durs.resize(nModes);
	}

	/* Functions to set states: */
#pragma region Setters
// Move on to the next state (returns True if end is reached)
	bool Next()
	{
		if (locked)
			return true;

		current++;
		int res = 0;
		for (int i = 0; i < nDims; ++i)
		{
			res = dims[i].next();
			if (res != -1)
				break;
		}

		return res == -1;
	}

	// Sets everything back to the starting state
	void Reset()
	{
		if (locked)
			return;

		for (int i = 0; i < dims.size(); ++i)
		{
			dims[i].setCurr(0);
			if (!dims[i].checkLocks())
				dims[i].next();
		}
	}

	// Sets the duration the current run took
	void SetCurrDur(double dur)
	{
		durs[current] = dur;
	}

	// Locks the setup into a given mode
	void LockMode(string modeName)
	{
		Reset();

		for (bool stop = false; !stop; stop = Next())
			if (GetCurrentModeName().compare(modeName) == 0)
			{
				locked = true;
				return;
			}

		cout << "ERROR: Locking mode failed; requested mode (" << modeName << ") not found!" << endl;
	}

	// Locks a dimension into a given setting
	void LockDim(string setName)
	{
		Reset();

		for (int i = 0; i < nDims; ++i)
		{
			int j;
			int max = dims[i].getMax();
			for (j = 0; j < max; ++j)
				if (dims[i].getModeName(j).compare(setName) == 0)
					break;

			if (j < max && dims[i].getModeName(j).compare(setName) == 0)
			{
				dims[i].lock(j);
				nModes /= dims[i].getMax();
				return;
			}
		}

		cout << "ERROR: Locking dim failed; requested setting (" << setName << ") not found!" << endl;
	}

	// Locks a specific setting into being ON or OFF
	void LockSetting(string setName, int val)
	{
		Reset();

		for (int i = 0; i < nDims; ++i)
		{
			int id = dims[i].getSetID(setName);
			if (id != -1)
			{
				dims[i].lockSetting(id, val);
				nModes /= 2;
				return;
			}
		}

		cout << "ERROR: Locking setting failed; requested setting (" << setName << ") not found!" << endl;
	}
#pragma endregion

	/* Functions to add Dimensions: */
#pragma region Add Dims
// Adds a named regular dimension 
	void AddDim(int max, string name)
	{
		dims.push_back(ModeDim(name, 0, max));
		updateCounters();
	}

	// Adds a regular dimension with each option named
	void AddDim(int max, string* names)
	{
		dims.push_back(ModeDim(names, 0, max));
		updateCounters();
	}

	// Adds a named combination dimension
	void AddCombDim(int max, string name)
	{
		dims.push_back(ModeDim(name, 1, max));
		updateCounters();
	}

	// Adds a combination dimension with each option named
	void AddCombDim(int max, string* names)
	{
		dims.push_back(ModeDim(names, 1, max));
		updateCounters();
	}

#pragma endregion

	/* Functions to get states: */
#pragma region Getters
// Get current mode for specific dimension
	int GetCurrent(int dim)
	{
		return dims[dim].getCurr();
	}

	// Get current mode for specific combination-dimension
	int GetCurrentComb(int dim, int index)
	{
		return dims[dim].getCurr(index);
	}

	// Get current value of a named setting
	int GetCurrentBySettingName(string name)
	{
		for (int i = 0; i < nDims; ++i)
		{
			int res = dims[i].getCurr(name);
			if (res != -1)
				return res;
		}
		return -1;
	}

	// Gets the ID of the current state
	int GetID()
	{
		return current;
	}

	// Get (mode)name of current mode
	string GetCurrentModeName()
	{
		string name = "";
		for (int i = 0; i < nDims; ++i)
			name += " " + dims[i].getModeName();
		return name.substr(1);
	}

	// Get (mode)name of current mode for specific dimension
	string GetCurrentModeName(int dim)
	{
		return dims[dim].getModeName();
	}

	// Get (setting)name of current mode for specific dimension
	string GetCurrentSettingName(int dim)
	{
		return dims[dim].getSetName();
	}

	// Get the number of different modes (i.e. adding a binary dimension DOUBLES number of modes)
	int GetNModes()
	{
		if (locked)
			return 1;
		return nModes;
	}

	// Get the number of different settings (i.e. adding a binary dimension ADDS TWO to the number of settings)
	int GetNSettings()
	{
		if (locked)
			return nDims;
		return nSettings;
	}

	// Get the duration a given mode took
	double GetDur(int mode)
	{
		return durs[mode];
	}

	// Gets the list of names for each mode
	vector<string> GetModeNames()
	{
		Reset();

		vector<string> res;

		for (bool stop = false; !stop; stop = Next())
			res.push_back(GetCurrentModeName());

		return res;
	}

	// Gets the list of names for each setting
	vector<string> GetSettingNames()
	{
		vector<string> res = vector<string>();

		for (int i = 0; i < nDims; ++i)
			for (int j = 0; j < dims[i].getSettings(); ++j)
				res.push_back(dims[i].getSetName(j));

		return res;
	}

	// Gets the list of names for each submode of a combmode
	vector<string> GetCombModeNames()
	{
		vector<string> res = vector<string>();

		for (int i = 0; i < nDims; ++i)
		{
			vector<string> curr = dims[i].getSubModeNames();
			res.insert(res.end(), curr.begin(), curr.end());
		}

		return res;
	}

	// Gets a list of bools indexed by setting to show which setting is on and which is off, in the current status
	vector<bool> GetSettingStatus()
	{
		vector<bool> res = vector<bool>();

		for (int i = 0; i < nDims; ++i)
		{
			vector<bool> curr = dims[i].getSetStat();
			res.insert(res.end(), curr.begin(), curr.end());
		}

		return res;
	}

	// Get a list of average durations per settings (to be run after all runs are completed)
	vector<double> GetSettingDurs()
	{
		Reset();

		vector<double> res(nSettings, 0.0);
		vector<int> counts(nSettings, 0);

		bool stop = false;

		for (int i = 0; i < nModes && !stop; ++i)
		{
			vector<bool> curs = GetSettingStatus();

			for (int j = 0; j < nSettings; ++j)
				if (curs[j])
				{
					res[j] += durs[i];
					counts[j] += 1;
				}

			stop = Next();
		}

		for (int i = 0; i < nSettings; ++i)
			res[i] = res[i] / counts[i];

		return res;
	}

	// Get a list of average durations per mode (to be run after all runs are completed)
	vector<double> GetModeDurs(vector<string> modeNames)
	{
		Reset();

		int size = modeNames.size();

		vector<double> res(size, 0.0);
		vector<int> counts(size, 0);

		bool stop = false;

		for (int i = 0; i < nModes && !stop; ++i)
		{
			for (int j = 0; j < nDims; ++j)
			{
				string name = GetCurrentModeName(j);
				int id = find(modeNames.begin(), modeNames.end(), GetCurrentModeName(j)) - modeNames.begin();

				if (id == size)
					continue;

				res[id] += durs[i];
				counts[id] += 1;
			}

			stop = Next();
		}

		for (int i = 0; i < size; ++i)
			res[i] = res[i] / counts[i];

		return res;
	}

	// Add other getters as needed
#pragma endregion
};